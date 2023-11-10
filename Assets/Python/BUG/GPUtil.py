## GPUtil
##
## Utilities for dealing with Great People.
##
## Notes
##   - Must be initialized externally by calling init()
##
## Copyright (c) 2007-2008 The BUG Mod.
##
## Author: EmperorFool

from CvPythonExtensions import *

gc = CyGlobalContext()
localText = CyTranslator()

# All seven GP types

GP_ADVENTURER = 0
GP_ARTIST = GP_ADVENTURER + 1
GP_COMMANDER = GP_ARTIST + 1
GP_ENGINEER = GP_COMMANDER + 1
GP_MERCHANT = GP_ENGINEER + 1
GP_PROPHET = GP_MERCHANT + 1
GP_SCIENTIST = GP_PROPHET + 1
NUM_GP = GP_SCIENTIST + 1

# List of GP to show in the GP Bars

g_gpBarList = (
				"UNIT_ADVENTURER",
				"UNIT_ARTIST",
				"UNIT_COMMANDER",
				"UNIT_ENGINEER",
				"UNIT_MERCHANT",
				"UNIT_PROPHET",
				"UNIT_SCIENTIST",
				)

# Maps GP type to unit ID and color to show in GP Bar (thus no GG here)

g_gpUnitTypes = None
g_gpColors = None
g_unitIcons = None

g_initDone = False

def init():
	global g_initDone
	if (g_initDone):
		return
		
	global g_gpUnitTypes
	g_gpUnitTypes = [None] * NUM_GP
	for i, s in enumerate(g_gpBarList):
		g_gpUnitTypes[i] = gc.getInfoTypeForString(s)
	
	global g_gpColors
	g_gpColors = [None] * NUM_GP
	g_gpColors[GP_ENGINEER] = gc.getYieldInfo(YieldTypes.YIELD_PRODUCTION).getColorType()
	g_gpColors[GP_MERCHANT] = gc.getInfoTypeForString("COLOR_YELLOW")
	g_gpColors[GP_SCIENTIST] = gc.getInfoTypeForString("COLOR_RESEARCH_STORED")
	g_gpColors[GP_ARTIST] = gc.getInfoTypeForString("COLOR_CULTURE_STORED")
	g_gpColors[GP_PROPHET] = gc.getInfoTypeForString("COLOR_BLUE")
	
	global g_unitIcons
	g_unitIcons = {}

	g_unitIcons[g_gpUnitTypes[GP_ADVENTURER]] = u"%c" %(CyGame().getSymbolID(FontSymbols.STRENGTH_CHAR))
	g_unitIcons[g_gpUnitTypes[GP_ARTIST]] = u"%c" %(gc.getCommerceInfo(CommerceTypes.COMMERCE_CULTURE).getChar())
	g_unitIcons[g_gpUnitTypes[GP_COMMANDER]] = u"%c" %(gc.getCommerceInfo(CommerceTypes.COMMERCE_ESPIONAGE).getChar())
	g_unitIcons[g_gpUnitTypes[GP_ENGINEER]] = u"%c" %(gc.getYieldInfo(YieldTypes.YIELD_PRODUCTION).getChar())
	g_unitIcons[g_gpUnitTypes[GP_MERCHANT]] = u"%c" %(gc.getCommerceInfo(CommerceTypes.COMMERCE_GOLD).getChar())
	g_unitIcons[g_gpUnitTypes[GP_PROPHET]] = u"%c" % CyGame().getSymbolID(FontSymbols.RELIGION_CHAR)
	g_unitIcons[g_gpUnitTypes[GP_SCIENTIST]] = u"%c" %(gc.getCommerceInfo(CommerceTypes.COMMERCE_RESEARCH).getChar())
	
	g_initDone = True
	
def getUnitType(gpType):
	return g_gpUnitTypes[gpType]

def getColor(gpType):
	return g_gpColors[gpType]

def getUnitIcon(iUnit):
	return g_unitIcons[iUnit]

def findNextCity():
	iMinTurns = None
	iTurns = 0
	pPlayer = gc.getPlayer(gc.getGame().getActivePlayer())
	iThreshold = pPlayer.greatPeopleThreshold(False)
	pBestCity = None
	
	for iCity in range(pPlayer.getNumCities()):
		pCity = pPlayer.getCity(iCity)
		if (pCity):
			iRate = pCity.getGreatPeopleRate()
			if (iRate > 0):
				iProgress = pCity.getGreatPeopleProgress()
				iTurns = (iThreshold - iProgress + iRate - 1) / iRate
				if (iMinTurns is None or iTurns < iMinTurns):
					iMinTurns = iTurns
					pBestCity = pCity
	return (pBestCity, iMinTurns)

def findMaxCity():
	iMaxProgress = 0
	pPlayer = gc.getPlayer(gc.getGame().getActivePlayer())
	pBestCity = None
	
	for iCity in range(pPlayer.getNumCities()):
		pCity = pPlayer.getCity(iCity)
		if (pCity):
			iProgress = pCity.getGreatPeopleProgress()
			if (iProgress > iMaxProgress):
				iMaxProgress = iProgress
				pBestCity = pCity
	return (pBestCity, iMaxProgress)

def getCityTurns(pCity):
	if (pCity):
		return pCity.getNextGreatPersonInfo(false)
	return None

def calcPercentages(pCity):
	# Calc total rate
	iTotal = 0
	for iUnit in g_gpUnitTypes:
		iTotal += pCity.getGreatPeopleUnitProgress(iUnit)
	
	# Calc individual percentages based on rates and total
	percents = []
	if (iTotal > 0):
		iLeftover = 100
		for iUnit in range(gc.getNumUnitInfos()):
#			iUnit = getUnitType(gpType)
			iProgress = pCity.getGreatPeopleUnitProgress(iUnit)
			if (iProgress > 0):
				iPercent = 100 * iProgress / iTotal
				iLeftover -= iPercent
				percents.append((iPercent, iUnit))
		# Add remaining from 100 to first in list to match Civ4
		if (iLeftover > 0):
			percents[0] = (percents[0][0] + iLeftover, percents[0][1])
	return percents

def createHoverText(pCity, iTurns):
	if (not pCity):
		return None
	iProgress = pCity.getGreatPeopleProgress()
	iThreshold = gc.getPlayer(pCity.getOwner()).greatPeopleThreshold(False)
	szText = localText.getText("TXT_KEY_MISC_GREAT_PERSON", (iProgress, iThreshold))
	iRate = pCity.getGreatPeopleRate()
	szText += u"\n"
	szText += localText.getText("INTERFACE_CITY_TURNS", (iTurns,))
	
	percents = calcPercentages(pCity)
	if (len(percents) > 0):
		percents.sort(reverse=True)
		for iPercent, iUnit in percents:
#			iUnit = getUnitType(gpType)
			szText += u"\n%s - %d%%" % (gc.getUnitInfo(iUnit).getDescription(), iPercent)
	return szText

def getGreatPeopleText(pCity, iGPTurns, iGPBarWidth, bGPBarTypesNone, bGPBarTypesOne, bIncludeCityName):
	sGreatPeopleChar = u"%c" % CyGame().getSymbolID(FontSymbols.GREAT_PEOPLE_CHAR)
	if (not pCity):
		szText = localText.getText("INTERFACE_GREAT_PERSON_NONE", (sGreatPeopleChar, ))
	elif (bGPBarTypesNone):
		if (iGPTurns):
			if (bIncludeCityName):
				szText = localText.getText("INTERFACE_GREAT_PERSON_CITY_TURNS", (sGreatPeopleChar, pCity.getName(), iGPTurns))
			else:
				szText = localText.getText("INTERFACE_GREAT_PERSON_TURNS", (sGreatPeopleChar, iGPTurns))
		else:
			if (bIncludeCityName):
				szText = localText.getText("INTERFACE_GREAT_PERSON_CITY", (sGreatPeopleChar, pCity.getName()))
			else:
				szText = sGreatPeopleChar
	else:
		lPercents = calcPercentages(pCity)
		if (iGPTurns):
			if (bIncludeCityName):
				szText = localText.getText("INTERFACE_GREAT_PERSON_CITY_TURNS", (sGreatPeopleChar, pCity.getName(), iGPTurns))
			else:
				szText = localText.getText("INTERFACE_GREAT_PERSON_TURNS", (sGreatPeopleChar, iGPTurns))
		else:
			if (bIncludeCityName):
				szText = localText.getText("INTERFACE_GREAT_PERSON_CITY", (sGreatPeopleChar, pCity.getName()))
			else:
				szText = sGreatPeopleChar
	return szText

def getSpellResearchText(iGPBarWidth):
	pPlayer=gc.getPlayer(CyGame().getActivePlayer())
	sGreatPeopleChar = u"%c" % gc.getCommerceInfo(CommerceTypes.COMMERCE_ARCANE).getChar()
	if pPlayer.getArcane()==0:
		szText = localText.getText("INTERFACE_GREAT_PERSON_NONE", (sGreatPeopleChar, ))

	lPercents = calcPercentagesSpellResearch(pPlayer)
	if (len(lPercents) == 0):
		szText = localText.getText("INTERFACE_GREAT_PERSON_TURNS", (sGreatPeopleChar, 0))
	else:
		lPercents.sort(reverse=True)
		if (len(lPercents) == 1):
			iPercent, i = lPercents[0]
			pInfo = gc.getManaschoolInfo(i)
			szText = sGreatPeopleChar + u":" + unicode(pInfo.getDescription())
			szText = szText + u"<font=2> 100%</font>"
		else:
			szText = sGreatPeopleChar + u":"
			szTypes = ""
			for iPercent, i in lPercents:
				szNewTypes = szTypes + u" %s %d%%" % (gc.getManaschoolInfo(i).getDescription(),iPercent)
				szNewText = szText + u"<font=2> %s</font>" % szTypes
				if (CyInterface().determineWidth(szNewText) > iGPBarWidth - 10):
					# Keep under width
					break
				szTypes = szNewTypes
			if (len(szTypes) > 0):
				szText += u"<font=2> %s</font>" % szTypes
	return szText

def calcPercentagesSpellResearch(pPlayer):
	# Calc total rate
	iTotal = 0
	for i in range(gc.getNumManaschoolInfos()):		
		iTotal += pPlayer.calculateArcaneManaSchoolProgressChance(i)
	
	# Calc individual percentages based on rates and total
	percents = []
	if (iTotal > 0):
		iLeftover = 100
		for i in range(gc.getNumManaschoolInfos()):		
			iProgress = pPlayer.calculateArcaneManaSchoolProgressChance(i)
			if (iProgress > 0):
				iPercent = 100 * iProgress / iTotal
				iLeftover -= iPercent
				percents.append((iPercent, i))
		# Add remaining from 100 to first in list to match Civ4
		if (iLeftover > 0):
			percents[0] = (percents[0][0] + iLeftover, percents[0][1])
	return percents	