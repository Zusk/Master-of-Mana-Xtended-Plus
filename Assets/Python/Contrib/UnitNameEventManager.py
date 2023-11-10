## UnitName
## by Ruff_Hi
## for BUG Mod
##-------------------------------------------------------------------
## Naming Convention
##  - ^civ4^ - no naming convention, uses standard civ4
##  - ^rd^ - random name
##  - ^rc^ - random civ related name
##  - ^ct^ - City
##  - ^cv^ - Civilization
##  - ^ut^ - unit (eg Archer)
##  - ^cb^ - combat type (Melee)
##  - ^dm^ - domain (Water)
##  - ^ld^ - leader
##  - ^cnt[f]^ - count across all units (increments based on unit)
##  - ^cntu[f]^ - count across same unit (increments based on unit)
##  - ^cntct[f]^ - count across same city (increments based on unit)
##  - ^cntuct[f]^ - count across same unit / city (increments based on unit)
##  - ^cntc[f]^ - count across same combat type (increments based on combat type)
##  - ^cntd[f]^ - count across same domain (increments based on domain)
##  - ^tt1[f][x:y]^ - total where the total is a random number between x and y (number)
##  - ^tt2[f][x]^ - total count (starts at x, incremented by 1 each time ^tt is reset to 1)
##
## Where [f] can be either 's', 'A', 'a', 'p', 'g', 'n', 'o' or 'r' for ...
##  - silent (not shown)
##  - alpha (A, B, C, D, ...)
##  - alpha (a, b, c, d, ...)
##  - phonetic (alpha, bravo, charlie, delta, echo, ...)
##  - greek (alpha, beta, gamma, delta, epsilon, ...)
##  - number
##  - ordinal (1st, 2nd, 3rd, 4th, ...)
##  - roman (I, IV, V, X, ...)
##
## Coding Steps
##
## 1. check if a unit exists, if not, do nothing
## 2. call unit name engine
## 3. update unit name if returned name is not NULL
##
## Unit name engine:
##
## 1. get naming convention from ini file
##    a. try to get the advanced naming convention
##    b. if it returns 'DEFAULT', then get the combat based naming convention
##    c. if naming convention is 'DEFAULT', get default naming convention
## 
## 2. determine if you have 'civ naming' or no valid naming codes in your naming convention, if YES, return 'NULL'
## 3. determine if you have 'random' in your naming convention, if YES, call random engine and return value
## 4. determine if you have 'random civ related' in your naming convention, if YES, call random civ related engine and return value
## 
## 5. swap out fixed items (ie unit, combat type, domain, leader, civilization, city, etc) from naming convention
## 
## 6. determine if you have any count items in naming convention; return if FALSE
## 
## 7. determine key for counting (this information is stored in the save file)
## a. get latest count from save (if not found, initilize)
## b. increment count by 1
## c. test against total (if required), adjust total and 2nd total if required
## 
## 8. format count items
## 
## 9. replace formatted count items in naming convention
## 
## 10. return name
##-------------------------------------------------------------------

from CvPythonExtensions import *
import CvUtil
import BugUtil
import PyHelpers
import BugPath
import BugConfigTracker
import BugCore
from configobj import ConfigObj
import Roman
import RandomNameUtils
import random
import Popup as PyPopup

#######SD Tool Kit#######

import SdToolKit
sdEcho			= SdToolKit.sdEcho
sdModInit		= SdToolKit.sdModInit
sdModLoad		= SdToolKit.sdModLoad
sdModSave		= SdToolKit.sdModSave
sdEntityInit	= SdToolKit.sdEntityInit
sdEntityExists	= SdToolKit.sdEntityExists
sdEntityWipe	= SdToolKit.sdEntityWipe
sdGetVal		= SdToolKit.sdGetVal
sdSetVal		= SdToolKit.sdSetVal
sdGroup			= "UnitCnt"

############################

RENAME_EVENT_ID = CvUtil.getNewEventID("UnitNaming.Rename")

gc = CyGlobalContext()
PyInfo = PyHelpers.PyInfo

UnitNamingOpt = BugCore.game.UnitNaming

phonetic_array = ['Alpha', 'Bravo', 'Charlie', 'Delta', 'Echo', 'Foxtrot', 'Golf', 'Hotel', 'India', 'Juliett', 'Kilo', 'Lima', 'Mike',
                  'November', 'Oscar', 'Papa', 'Quebec', 'Romeo', 'Sierra', 'Tango', 'Uniform', 'Victor', 'Whiskey', 'X-Ray', 'Yankee', 'Zulu']

greek_array = ['Alpha', 'Beta', 'Gamma', 'Delta', 'Epsilon', 'Zeta', 'Eta', 'Theta', 'Iota', 'Kappa', 'Lambda', 'Mu', 'Nu', 'Xi',
               'Omicron', 'Pi', 'Rho', 'Sigma', 'Tau', 'Upsilon', 'Phi', 'Chi', 'Psi', 'Omega']

ordinal_array = 'th st nd rd th th th th th th'.split()

def BUGPrint (stuff):
#	stuff = "UNEvMg: " + stuff
#	print stuff
	return

class UnitNameEventManager:

	def __init__(self, eventManager):

		BuildUnitName(eventManager)
		
		# additions to self.Events
		moreEvents = {
			RENAME_EVENT_ID : ('', self.__eventUnitRenameApply,  self.__eventUnitRenameBegin),
		}
		eventManager.Events.update(moreEvents)
		self.eventMgr = eventManager

		self.UnitNameConv = "^ut^ ^cntu[n]^ of ^ct^"
		self.Prompt = "Enter a rename convention"

	def __eventUnitRenameBegin(self, argsList):
		header = "Unit Name Testing (cancel to quit)"  #BugUtil.getPlainText("TXT_KEY_REMINDER_HEADER")
		prompt = self.Prompt   #"Enter a rename convention"   #BugUtil.getPlainText("TXT_KEY_REMINDER_PROMPT")
		ok = BugUtil.getPlainText("TXT_KEY_MAIN_MENU_OK")
		cancel = BugUtil.getPlainText("TXT_KEY_POPUP_CANCEL")
		popup = PyPopup.PyPopup(RENAME_EVENT_ID, EventContextTypes.EVENTCONTEXT_SELF)
		popup.setHeaderString(header)
		popup.setBodyString(prompt)
		popup.createPythonEditBox(self.UnitNameConv, "Enter the unit name convention that you want to test.", 0)
#		popup.createPythonCheckBoxes(1, 0)
#		popup.setPythonCheckBoxText(0, "Check to increment counters", "Note: if checked, units named in-game commence from counter used in testing.", 0)
		popup.addButton("Ok, don't increment counter")
		popup.addButton("Ok, increment counter")
		popup.addButton(cancel)
		popup.launch(False, PopupStates.POPUPSTATE_IMMEDIATE)

#				popup.createPythonCheckBoxes(3, 0)
#				for i in range(3):
#					strCheckboxText = "Checkbox " + str(i)
#					strCheckBoxHelp = "Checkbox Help Text " + str(i)
#					# set checkbox text of button i, group 0 to strCheckboxText
#					popup.setPythonCheckBoxText(i, strCheckboxText, strCheckBoxHelp, 0)

	def __eventUnitRenameApply(self, playerID, userData, popupReturn):
		self.testUnitNameConv(popupReturn)

	def testUnitNameConv(self, popupReturn):

		if (popupReturn.getButtonClicked() == 2):
			return

		pPlayer = gc.getActivePlayer()
		pUnit = pPlayer.getUnit(0)
		pCity = pPlayer.getCity(0)
		lUnitReName = UnitReName()

		zsEra = gc.getEraInfo(pPlayer.getCurrentEra()).getType()
		zsUnitCombat = lUnitReName.getUnitCombat(pUnit)
		zsUnitClass = gc.getUnitClassInfo(pUnit.getUnitClassType()).getType()

#		BUGPrint("ERA(%s)" % (zsEra))
#		BUGPrint("Combat(%s)" % (zsUnitCombat))
#		BUGPrint("Class(%s)" % (zsUnitClass))

		zsUnitNameConv = lUnitReName.getUnitNameConvFromIniFile(zsEra, zsUnitClass, zsUnitCombat)
		zsUnitNameConv = popupReturn.getEditBoxString(0)
		self.UnitNameConv = zsUnitNameConv

		zsUnitName = lUnitReName.getUnitName(zsUnitNameConv, pUnit, pCity, popupReturn.getButtonClicked() == 1)

		self.Prompt = "Using the convention\n   '%s'\ngenerated the unit name\n   '%s'\n\nEnter another rename convention" % (zsUnitNameConv, zsUnitName)

		self.eventMgr.beginEvent(RENAME_EVENT_ID)
		return





class AbstractBuildUnitName(object):

	def __init__(self, eventManager, *args, **kwargs):
		super(AbstractBuildUnitName, self).__init__(*args, **kwargs)

class BuildUnitName(AbstractBuildUnitName):

	def __init__(self, eventManager, *args, **kwargs):
		super(BuildUnitName, self).__init__(eventManager, *args, **kwargs)

		eventManager.addEventHandler("kbdEvent", self.onKbdEvent)
		eventManager.addEventHandler("unitBuilt", self.onUnitBuilt)

		self.eventMgr = eventManager
		self.config = None

	def onKbdEvent(self, argsList):
		eventType,key,mx,my,px,py = argsList
		if ( eventType == self.eventMgr.EventKeyDown ):
			if (int(key) == int(InputTypes.KB_N)
			and self.eventMgr.bCtrl
			and self.eventMgr.bAlt):

				if UnitNamingOpt.isEnabled():
					self.eventMgr.beginEvent(RENAME_EVENT_ID)

		return 0

	def onUnitBuilt(self, argsList):
		'Unit Completed'

		pCity = argsList[0]
		pUnit = argsList[1]
		iPlayer = pUnit.getOwner()
		pPlayer = gc.getPlayer(iPlayer)
		lUnitReName = UnitReName()

#		BUGPrint("onUnitBuild-A")

		if (pUnit == None
		or pUnit.isNone()):
			return

#		BUGPrint("onUnitBuild-B %s %s %s" % (iPlayer, CyGame().getActivePlayer(), UnitNamingOpt.isEnabled()))

		if not (iPlayer == CyGame().getActivePlayer()
		and UnitNamingOpt.isEnabled()):
			return

#		BUGPrint("onUnitBuild-C")

		zsEra = gc.getEraInfo(pPlayer.getCurrentEra()).getType()
		zsUnitCombat = lUnitReName.getUnitCombat(pUnit)
		zsUnitClass = gc.getUnitClassInfo(pUnit.getUnitClassType()).getType()

#		BUGPrint("ERA(%s)" % (zsEra))
#		BUGPrint("Combat(%s)" % (zsUnitCombat))
#		BUGPrint("Class(%s)" % (zsUnitClass))

		zsUnitNameConv = lUnitReName.getUnitNameConvFromIniFile(zsEra, zsUnitClass, zsUnitCombat)
		zsUnitName = lUnitReName.getUnitName(zsUnitNameConv, pUnit, pCity, True)

#		BUGPrint("onUnitBuild-D")

		if not (zsUnitName == ""):
			pUnit.setName(zsUnitName)

#		BUGPrint("onUnitBuild-E")

		return




class UnitReName(object):

	def getUnitName(self, sUnitNameConv, pUnit, pCity, bIncrementCounter):

		iPlayer = pUnit.getOwner()
		pPlayer = gc.getPlayer(iPlayer)
		
		zsEra = gc.getEraInfo(pPlayer.getCurrentEra()).getType()
		zsCiv = gc.getPlayer(iPlayer).getCivilizationShortDescription(0)
		zsLeader = gc.getPlayer(iPlayer).getName()
		zsUnitCombat = self.getUnitCombat(pUnit)
		zsUnitClass = gc.getUnitClassInfo(pUnit.getUnitClassType()).getType()
		zsUnitType = gc.getUnitInfo(pUnit.getUnitType()).getType()
		zsUnitDomain = gc.getDomainInfo(pUnit.getDomainType()).getType()
		zsUnit = PyInfo.UnitInfo(pUnit.getUnitType()).getDescription()
		zsCity = pCity.getName()

#		BUGPrint("ERA(%s)" % (zsEra))
#		BUGPrint("Civ(%s)" % (zsCiv))
#		BUGPrint("Leader(%s)" % (zsLeader))
#		BUGPrint("Combat(%s)" % (zsUnitCombat))
#		BUGPrint("Class(%s)" % (zsUnitClass))
#		BUGPrint("Type(%s)" % (zsUnitType))
#		BUGPrint("Domain(%s)" % (zsUnitDomain))
#		BUGPrint("Unit(%s)" % (zsUnit))
#		BUGPrint("City(%s)" % (zsCity))

		zsName = sUnitNameConv

		#if zsName == "":
		#zsName = "^ut^ ^cnt[r]^ Div ^tt1[s][5:7]^ : ^ct^ ^tt2[o][101]^"

#		BUGPrint("UnitNameEM-A [" + zsName + "]")

##  - ^civ4^ - no naming convention, uses standard civ4
#		check if Civ4 naming convention is required
		if not (zsName.find("^civ4^") == -1):
			return ""

##  - ^rd^ - random name
#		check if random naming convention is required
		if not (zsName.find("^rd^") == -1):
			return RandomNameUtils.getRandomName()

#		BUGPrint("UnitNameEM-B")

##  - ^rc^ - random civ related name
#		check if random civ related naming convention is required
		if not (zsName.find("^rc^") == -1):
			return RandomNameUtils.getRandomCivilizationName(pPlayer.getCivilizationType())

#		BUGPrint("UnitNameEM-C [" + zsName + "]")

##  - ^ct^ - City
##  - ^cv^ - Civilization
##  - ^ut^ - unit (eg Archer)
##  - ^cb^ - combat type (Melee)
##  - ^dm^ - domain (Water)
##  - ^ld^ - leader
#		replace the fixed items in the naming conv
		zsName = zsName.replace("^ct^", zsCity)
		zsName = zsName.replace("^cv^", zsCiv)
		zsName = zsName.replace("^ut^", zsUnit)
		zsName = zsName.replace("^cb^", zsUnitCombat)
		zsName = zsName.replace("^dm^", zsUnitDomain)
		zsName = zsName.replace("^ld^", zsLeader)

#		BUGPrint("UnitNameEM-D [" + zsName + "]")

#		check if there are any more codes to swap out, return if not
		if (zsName.find("^") == -1):
			return zsName

#		determine what I am counting across
		zsSDKey = self.getCounter(zsName)
		if zsSDKey == "UNIT":		zsSDKey = zsSDKey + zsUnit
		elif zsSDKey == "COMBAT":	zsSDKey = zsSDKey + zsUnitCombat
		elif zsSDKey == "CITY":		zsSDKey = zsSDKey + zsCity
		elif zsSDKey == "UNITCITY": zsSDKey = zsSDKey + zsUnit + zsCity
		elif zsSDKey == "DOMAIN":	zsSDKey = zsSDKey + zsUnitDomain

#		BUGPrint("UnitNameEM-E [" + zsSDKey + "]")

#		see if we have already started this counter
		if (sdEntityExists(sdGroup, zsSDKey) == False):
			#Since no record create entries
			ziTT1 = self.getTotal1(zsName)
			ziTT2 = self.getTotal2(zsName)
			zDic = {'cnt':0, 'tt1':ziTT1, 'tt2':ziTT2}
			sdEntityInit(sdGroup, zsSDKey, zDic)

#		get the count values
		ziCnt = sdGetVal(sdGroup, zsSDKey, "cnt")
		ziTT1 = sdGetVal(sdGroup, zsSDKey, "tt1")
		ziTT2 = sdGetVal(sdGroup, zsSDKey, "tt2")

#		BUGPrint("UnitNameEM-F [" + str(ziCnt) + "] [" + str(ziTT1) + "] [" + str(ziTT2) + "]")

#		increment count, adjust totals if required
		if bIncrementCounter:
			ziCnt = ziCnt + 1
			if (ziCnt > ziTT1
			and ziTT1 > 0):
				ziCnt = 1
				ziTT1 = self.getTotal1(zsName)
				ziTT2 = ziTT2 + 1

#		store the new values
		sdSetVal(sdGroup, zsSDKey, "cnt", ziCnt)
		sdSetVal(sdGroup, zsSDKey, "tt1", ziTT1)
		sdSetVal(sdGroup, zsSDKey, "tt2", ziTT2)

#		swap out the count code items for count value
		zsName = self.swapCountCode(zsName, "^cnt", ziCnt)
		zsName = self.swapCountCode(zsName, "^tt1", ziTT1)
		zsName = self.swapCountCode(zsName, "^tt2", ziTT2)

		return zsName
	
	def getUnitNameConvFromIniFile(self, Era, UnitClass, UnitCombat):
##    a. try to get the advanced naming convention
##    b. if it returns 'DEFAULT', then get the combat based naming convention
##    c. if naming convention is 'DEFAULT', get default naming convention

		if UnitNamingOpt.isAdvanced():
			era = Era[4:]
			unitClass = UnitClass[10:]
			zsUnitNameConv = UnitNamingOpt.getByEraAndClass(era, unitClass)
		else:
			zsUnitNameConv = "DEFAULT"

		if not (zsUnitNameConv == "DEFAULT"):
			return zsUnitNameConv

#		BUGPrint("UnitNameEM-iniA [" + zsUnitNameConv + "]" + UnitCombat[11:])

		zsUnitNameConv = UnitNamingOpt.getByCombatType(UnitCombat[11:])

#		BUGPrint("UnitNameEM-iniB [" + zsUnitNameConv + "]")

		if not (zsUnitNameConv == "DEFAULT"):
			return zsUnitNameConv

#		BUGPrint("UnitNameEM-iniC [" + zsUnitNameConv + "]")

		zsUnitNameConv = UnitNamingOpt.getDefault()
		return zsUnitNameConv


	def getUnitCombat(self, pUnit):

# Return immediately if the unit passed in is invalid
		if (pUnit == None
		or pUnit.isNone()):
			return "UNITCOMBAT_None"

		iUnitCombat = pUnit.getUnitCombatType()
		infoUnitCombat = gc.getUnitCombatInfo(iUnitCombat)

		if (infoUnitCombat == None):
			return "UNITCOMBAT_None"

		return infoUnitCombat.getType()


	def getCounter(self, conv):
##  - ^cnt[f]^ - count across all units (increments based on unit)
##  - ^cntu[f]^ - count across same unit (increments based on unit)
##  - ^cntct[f]^ - count across same city (increments based on unit)
##  - ^cntuct[f]^ - count across same unit / city (increments based on unit)
##  - ^cntc[f]^ - count across same combat type (increments based on combat type)
##  - ^cntd[f]^ - count across same domain (increments based on domain)

		if not (conv.find("^cnt[") == -1):
			return "ALL"

		if not (conv.find("^cntu[") == -1):
			return "UNIT"

		if not (conv.find("^cntc[") == -1):
			return "COMBAT"

		if not (conv.find("^cntct[") == -1):
			return "CITY"

		if not (conv.find("^cntuct[") == -1):
			return "UNITCITY"

		if not (conv.find("^cntd[") == -1):
			return "DOMAIN"

		return "ALL"


	def getTotal1(self, conv):
##  - ^tt1[f][x:y]^ - total where the total is a random number between x and y (number)

#		return 'not found' indicator
		ziStart = conv.find("^tt1[")
		if (ziStart == -1):
			return -1

#		locate and extract the 'low' value
		ziStart = conv.find("[",ziStart)
		ziStart = conv.find("[",ziStart + 1)
		ziEnd = conv.find(":",ziStart)
		ziLow = int(conv[ziStart + 1:ziEnd])
		if (ziLow < 1): ziLow = 1

#		locate and extract the 'high' value
		ziStart = ziEnd
		ziEnd = conv.find("]",ziStart)
		ziHigh = int(conv[ziStart + 1:ziEnd])
		if (ziHigh < 1): ziHigh = 1

#		check that the user isn't an idiot
		if (ziLow > ziHigh): return ziLow

#		return the value
		return random.randint(ziLow, ziHigh)


	def getTotal2(self, conv):
##  - ^tt2[f][x]^ - total count (starts at x, incremented by 1 each time ^tt is reset to 1)

#		return 'not found' indicator
		ziStart = conv.find("^tt2[")
		if (ziStart == -1):
			return -1

#		locate and extract the value
		ziStart = conv.find("[",ziStart)
		ziStart = conv.find("[",ziStart + 1)
		ziEnd = conv.find("]",ziStart)
		ziValue = int(conv[ziStart + 1:ziEnd])

		if (ziValue < 1): ziValue = 1
		return ziValue


	def getNumberFormat(self, conv, searchStr):
#		return 'not found' indicator
		ziStart = conv.find(searchStr)
		ziStart = conv.find("[",ziStart)
		if (ziStart == -1):
			return "s"   # s for silent, hides number
		else:
			return conv[ziStart + 1:ziStart + 2]


	def getCountCode(self, conv, searchStr):
#		return 'not found' indicator
		ziStart = conv.find(searchStr)
		if (ziStart == -1):
			return ""
		else:
			ziEnd = conv.find("^", ziStart + 1)
			return conv[ziStart:ziEnd + 1]


	def swapCountCode(self, conv, searchStr, iCnt):

#		return if iCnt is negative (this means that the code is not in the unitnameconv)
		if iCnt < 0: return conv

#		BUGPrint("UnitNameEM-SCC [" + conv + "] [" + searchStr + "] [" + str(iCnt) + "]")

		zsCntCode = self.getCountCode(conv, searchStr)

		if zsCntCode == "": return conv

#		BUGPrint("UnitNameEM-SCC [" + zsCntCode + "]")

		zsNumberFormat = self.getNumberFormat(conv, searchStr)

#		BUGPrint("UnitNameEM-SCC [" + zsNumberFormat + "]")

		zsCnt = self.FormatNumber(zsNumberFormat, iCnt)

#		BUGPrint("UnitNameEM-SCC [" + zsCnt + "]")

		if zsCntCode == "":
			return conv
		else:
			return conv.replace(zsCntCode, zsCnt)


	def FormatNumber(self, fmt, i):
		if (fmt == "s"):     # silent
			return ""
		elif (fmt == "a"):   # lower case alpha
			i = ((i + 1) % 26) - 1
			return chr(96+i)
		elif (fmt == "A"):   # upper case alpha
			i = ((i + 1) % 26) - 1
			return chr(64+i)
		elif (fmt == "p"):   # phonetic
			i = ((i + 1) % 26) - 1
			return phonetic_array[i]
		elif (fmt == "g"):   # greek
			i = ((i + 1) % 24) - 1
			return greek_array[i]
		elif (fmt == "n"):   # number    
			return str(i)
		elif (fmt == "o"):   # ordinal
			return self.getOrdinal(i)
		elif (fmt == "r"):   # roman
			return Roman.toRoman(i)
		else:
			return str(i)


	def getOrdinal(self, i):
		if i % 100 in (11, 12, 13): #special case
			return '%dth' % i
		return str(i) + ordinal_array[i % 10]
