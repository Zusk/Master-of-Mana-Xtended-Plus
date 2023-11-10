#-------------------------------------------------------------------------------
# Name:        CvStrategyOverlay.py
# Purpose:     Draws the strategy overlay itself.
#              CvOverlayScreen.py does the editing.
#              Contains:
#              -Dot Mapper
#              -Categorized signs
#
# Author:      Del69, EmperorFool
#
# Created:     11/12/2008
#-------------------------------------------------------------------------------

from CvPythonExtensions import *
from SdToolkit import *
import BugCore
import BugUtil
import ColorUtil
import CvOverlayScreenUtils
import PlayerUtil

gc = CyGlobalContext()
StratLayerOpt = BugCore.game.StrategyOverlay

g_layers = {}

def init():
	DotMapLayer()

def getLayer(id):
	return g_layers[id]

def onGameStart(argsList):
	for layer in g_layers.itervalues():
		layer.reset()

def onLoad(argsList):
	for layer in g_layers.itervalues():
		layer.read()

def onPreSave(argsList):
	for layer in g_layers.itervalues():
		layer.write()

def onEnabledOptionChanged(option, value):
	pass

class StrategyLayer(object):
	"""
	Provides common functionality for all of the strategy layers.
	"""
	def __init__(self, id):
		self.MOD_SAVE_ID = "StrategyOverlay"
		self.id = id
		self.visible = False
		self.editing = False
		self.dirty = False
		g_layers[id] = self
		self.reset()
	
	def reset(self):
		"""
		Resets the data to a blank state and clears the dirty flag.
		"""
		self.dirty = False
	
	def read(self):
		"""
		Reads the data from the game and clears the dirty flag.
		"""
		self.dirty = False
	
	def write(self):
		"""
		Writes the data to the game and clears the dirty flag.
		"""
		self.dirty = False
	
	def toggleVisibility(self):
		if self.visible:
			self.hide()
		else:
			self.show()
	
	def show(self):
		if not self.visible:
			self.visible = True
			return True
		return False
	
	def hide(self):
		if self.visible:
			self.freeze()
			self.visible = False
			return True
		return False
	
	def toggleEditing(self):
		if not self.editing:
			self.edit()
		else:
			self.freeze()
	
	def edit(self):
		if not self.editing:
			self.show()
			self.editing = True
			return True
		return False
	
	def freeze(self):
		if self.editing:
			self.editing = False
			return True
		return False


## ----------------------------------------------------------------------
## DOT MAP
## ----------------------------------------------------------------------

DOTMAP_LAYER = "DotMap"
X, Y = 0, 1    # used in point tuples instead of creating a new class

g_DotMap = None

class City:
	"""
	Holds the data for a single dot-mapped city.
	"""
	def __init__(self, point, color, layer):
		self.point = point
		self.color = color
		self.layer = layer
	
	def __eq__(self, other):
		return self.point == other.point and self.color == other.color
	
	def __str__(self):
		return "(%d,%d) on %d" % (self.point[X], self.point[Y], self.layer)
	
	def isAt(self, point):
		return self.point == point
	
	def samePoint(self, other):
		return self.point == other.point
	
	def sameColor(self, other):
		return self.color == other.color
	
	def sameLayer(self, other):
		return self.layer == other.layer

def getDotMap():
	global g_DotMap
	if g_DotMap is None:
		BugUtil.error("CvStrategyOverlay has not been initialized")
	return g_DotMap

def toggleDotMapVisibility(args):
	g_DotMap.toggleVisibility()
	StratLayerOpt.setShowDotMap(g_DotMap.visible)

def toggleDotMapEditMode(args):
	g_DotMap.toggleEditing()
	if not g_DotMap.editing and not StratLayerOpt.isShowDotMap():
		g_DotMap.hide()

def onShowDotMapOptionChanged(option, value):
	if value:
		g_DotMap.show()
	else:
		g_DotMap.hide()

def onDotMapOptionChanged(option, value):
	g_DotMap.optionChanged(option, value)

class DotMapLayer(StrategyLayer):
	"""
	Draws city crosses of different colors so the user can create a dot-map.
	"""
	def __init__(self):
		super(DotMapLayer, self).__init__(DOTMAP_LAYER)
		global g_DotMap
		g_DotMap = self
		# constants
		self.CITY_SAVE_ID = "CityDataDict"
		self.HIGHLIGHT_CROSS_LAYER = 8
		self.FIRST_CROSS_LAYER = 9
		self.NUM_CROSS_LAYERS = len(ColorUtil.getColorKeys())
		self.DOT_LAYER = PlotLandscapeLayers.PLOT_LANDSCAPE_LAYER_NUMPAD_HELP
		self.NO_DOT_STYLE = PlotStyles.PLOT_STYLE_NONE
		self.MAX_DOT_STYLE = PlotStyles.PLOT_STYLE_WAVES
		self.BFC_OFFSETS = []
		for x in range(-2, 3):
			for y in range(-2, 3):
				if abs(x) != 2 or abs(y) != 2:
					self.BFC_OFFSETS.append((x, y))
		# default options
		self.CROSS_ALPHA = 50.0
		self.DOT_ALPHA = 50.0
		self.HIGHLIGHT_CROSS_ALPHA = 100.0
		self.HIGHLIGHT_DOT_ALPHA = 100.0
		self.DRAW_DOTS = True
		self.DOT_STYLE = PlotStyles.PLOT_STYLE_DOT_TARGET
		self.readOptions()
		# state
		self.highlightedCity = None
	
	def reset(self):
		self.cities = {}
		self.dirty = False
	
	def read(self):
		data = sdGetGlobal(self.MOD_SAVE_ID, self.CITY_SAVE_ID)
		if data is not None:
			self.cities = self.updateData(data)
			self.dirty = False
		else:
			self.reset()
#			player = gc.getPlayer(0)
#			city = player.getCity(0)
#			x = city.getX()
#			y = city.getY()
#			self.addCityAt((x + 1, y + 2), 2, self.FIRST_CROSS_LAYER)
	
	def updateData(self, data):
		"""
		Upgrade previous data formats to latest format.
		"""
		if len(data) == 0:
			# empty, don't care
			return data
		for key, value in data.iteritems():
			if isinstance(key, int):
				# data in latest format
				return data
			else:
				break
		# find first living, human player and assign all data to them
		# if none found, assign to player 0
		for player in PlayerUtil.players(alive=True, human=True):
			ePlayer = player.getID()
			break
		else:
			ePlayer = 0
		newData = {}
		cities = {}
		newData[ePlayer] = cities
		for point, colorLayer in data.iteritems():
			cities[point] = City(point, colorLayer[0], colorLayer[1])
		return newData
		
	def write(self):
		if self.dirty:
			sdSetGlobal(self.MOD_SAVE_ID, self.CITY_SAVE_ID, self.cities)
			self.dirty = False
	
	def show(self):
		if super(DotMapLayer, self).show():
			self.redrawCities()
	
	def hide(self):
		if super(DotMapLayer, self).hide():
			self.clearCityLayers()
	
	def edit(self):
		if super(DotMapLayer, self).edit():
			CvOverlayScreenUtils.showOverlayScreen()
	
	def freeze(self):
		if super(DotMapLayer, self).freeze():
			self.unhighlightCity()
			CvOverlayScreenUtils.hideOverlayScreen()
	
	
	def hasCities(self, ePlayer):
		return ePlayer in self.cities
	
	def hasCity(self, ePlayer, point):
		return self.hasCities(ePlayer) and point in self.cities[ePlayer]
	
	def getCities(self, ePlayer):
		if self.hasCities(ePlayer):
			return self.cities[ePlayer]
		cities = {}
		self.cities[ePlayer] = cities
		return cities
	
	def getCity(self, ePlayer, point):
		if self.hasCities(ePlayer):
			cities = self.cities[ePlayer]
			if point in cities:
				return cities[point]
		return None
	
	def iterCities(self, ePlayer):
		"""
		Iterates over the player's cities.
		"""
		if self.hasCities(ePlayer):
			for city in self.getCities(ePlayer).itervalues():
				yield city
	
	def addCityAt(self, point, color, layer):
		"""
		Adds the city to the data set and draws its dot and cross.
		"""
		city = City(point, color, layer)
		self.addCity(city)
	
	def addCity(self, city):
		"""
		Adds the city to the data set and draws its dot and cross.
		"""
		ePlayer = PlayerUtil.getActivePlayerID()
		if self.hasCity(ePlayer, city.point):
			oldCity = self.getCity(ePlayer, city.point)
			if city == oldCity:
				return
			self.removeCity(oldCity)
		BugUtil.debug("DotMap - adding city %s", city)
		self.getCities(ePlayer)[city.point] = city
		self.dirty = True
		self.drawCity(city, self.CROSS_ALPHA, self.DOT_ALPHA)
		
	def removeCityAt(self, point):
		"""
		Removes the city from the data set and erases its dot and cross.
		"""
		self.removeCity(self.getCity(PlayerUtil.getActivePlayerID(), point))
		
	def removeCity(self, city):
		"""
		Removes the city from the data set and erases its dot and cross.
		"""
		if city:
			BugUtil.debug("DotMap - removing city %s", city)
			ePlayer = PlayerUtil.getActivePlayerID()
			if self.hasCity(ePlayer, city.point):
				del self.getCities(ePlayer)[city.point]
				self.dirty = True
				self.redrawCrosses(city.layer)
				self.eraseDot(city, self.DOT_ALPHA)
			else:
				BugUtil.debug("City doesn't exist")
		else:
			self.freeze()
	
	def highlightCity(self, point, color):
		"""
		Highlights the given city location by drawing it using the given color on the highlight layer.
		Unhighlights the currently highlighted city if there is one.
		
		If there is no city there (N), the new city is drawn (C).
		If the city is on the same layer (S), nothing is done (N). --> WC
		If the city is on a different layer (D), the city's layer is redrawn without it (W) and the new city is drawn (C).
		"""
		city = City(point, color, self.HIGHLIGHT_CROSS_LAYER)
		if self.highlightedCity:
			if self.highlightedCity == city:
				return
			else:
				self.unhighlightCity()
		self.highlightedCity = city
		ePlayer = PlayerUtil.getActivePlayerID()
		existingCity = self.getCity(ePlayer, point)
		if existingCity is not None:
			self.redrawCrosses(existingCity.layer, point)
			self.eraseDot(existingCity, self.DOT_ALPHA)
		self.drawCross(city, self.HIGHLIGHT_CROSS_ALPHA)

	def unhighlightCity(self):
		"""
		Removes the highlight from the existing city location if there is one.
		
		If there is no city there (N), the current layer is redrawn (L) and the dot is erased (d).
		If the city is on the same layer (S), nothing is done (N). --> LC
		If the city is on a different layer (D), the current layer is redrawn (L) and the city is drawn (C).
		"""
		if self.highlightedCity:
			point = self.highlightedCity.point
			self.clearHighlightCrossLayer()
			ePlayer = PlayerUtil.getActivePlayerID()
			city = self.getCity(ePlayer, point)
			if city is not None:
				self.drawCity(city, self.CROSS_ALPHA, self.DOT_ALPHA)
			self.highlightedCity = None
	
	def redrawCities(self):
		"""
		Erases all city layers and draws all of the cities.
		"""
		self.clearCityLayers()
		self.drawCities()
	
	def redrawCrosses(self, layer, skip=None):
		"""
		Erases the given layer and draws all city crosses in that layer.
		"""
		self.clearCrossLayer(layer)
		self.drawCrosses(layer, skip)
	
	def redrawDots(self):
		"""
		Erases and redraws all city dots as they are all in the same layer.
		"""
		self.clearDotLayer()
		self.drawDots()
	
	def drawCities(self, skip=None):
		"""
		Draws all of the cities except skip, if given.
		"""
		crossAlpha = self.CROSS_ALPHA
		dotAlpha = self.DOT_ALPHA
		for city in self.iterCities(PlayerUtil.getActivePlayerID()):
			if not city.isAt(skip):
				self.drawCity(city, crossAlpha, dotAlpha)
	
	def drawCrosses(self, layer=None, skip=None):
		"""
		Draws the cross for every city in the given layer.
		"""
		crossAlpha = self.CROSS_ALPHA
		for city in self.iterCities(PlayerUtil.getActivePlayerID()):
			if not city.isAt(skip):
				if layer is None or layer == city.layer:
					self.drawCross(city, crossAlpha)
	
	def drawDots(self, skip=None):
		"""
		Draws the dot for every city.
		"""
		dotAlpha = self.DOT_ALPHA
		for city in self.iterCities(PlayerUtil.getActivePlayerID()):
			if not city.isAt(skip):
				self.drawDot(city, dotAlpha)
	
	def drawCity(self, city, crossAlpha, dotAlpha):
		"""
		Draws the cross and dot for a single city.
		"""
		self.drawCross(city, crossAlpha)
		self.drawDot(city, dotAlpha)
	
	def drawCross(self, city, alpha):
		"""
		Draws the cross for a single city.
		"""
		x, y = city.point
		color = ColorUtil.indexToKey(city.color)
		layer = city.layer
		for dx, dy in self.BFC_OFFSETS:
			CyEngine().fillAreaBorderPlotAlt(x + dx, y + dy, layer, color, alpha)
	
	def drawDot(self, city, alpha):
		"""
		Draws the dot for a single city.
		"""
		if self.DRAW_DOTS:
			x, y = city.point
			color = ColorUtil.indexToKey(city.color)
			CyEngine().addColoredPlotAlt(x, y, self.DOT_STYLE, self.DOT_LAYER, color, alpha)
	
	def eraseDot(self, city, alpha):
		"""
		Erases the dot for a single city.
		"""
		if self.DRAW_DOTS:
			x, y = city.point
			CyEngine().addColoredPlotAlt(x, y, self.NO_DOT_STYLE, self.DOT_LAYER, "COLOR_BLACK", alpha)
	
	def clearCityLayers(self):
		"""
		Erases all city crosses and dots.
		"""
		self.clearHighlightCrossLayer()
		for index in range(self.NUM_CROSS_LAYERS):
			self.clearCrossLayer(index + self.FIRST_CROSS_LAYER)
		self.clearDotLayer()

	def clearHighlightCrossLayer(self):
		"""
		Clears the indexed border layer.
		"""
		self.clearCrossLayer(self.HIGHLIGHT_CROSS_LAYER)

	def clearCrossLayer(self, layer):
		"""
		Clears the indexed border layer.
		"""
		CyEngine().clearAreaBorderPlots(layer)

	def clearDotLayer(self):
		"""
		Clears all the dots from screen.
		"""
		CyEngine().clearColoredPlots(self.DOT_LAYER)
	
	def percentToAlpha(self, percent):
		return min(100, max(0, percent)) / 100.0
	
	def readOptions(self):
		self.CROSS_ALPHA = self.percentToAlpha(StratLayerOpt.getDotMapBrightness())
		self.DOT_ALPHA = self.percentToAlpha(StratLayerOpt.getDotMapBrightness())
		self.HIGHLIGHT_CROSS_ALPHA = self.percentToAlpha(StratLayerOpt.getDotMapHighlightBrightness())
		self.HIGHLIGHT_DOT_ALPHA = self.percentToAlpha(StratLayerOpt.getDotMapHighlightBrightness())
		self.DRAW_DOTS = StratLayerOpt.isDotMapDrawDots()
		self.DOT_STYLE = min(self.MAX_DOT_STYLE, max(0, StratLayerOpt.getDotMapDotIcon()))
	
	def optionChanged(self, option, value):
		"""
		Redraws the layer if it is currently visible.
		"""
		self.unhighlightCity()
		self.readOptions()
		if self.visible:
			self.redrawCities()
