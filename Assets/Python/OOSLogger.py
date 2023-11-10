import os
import codecs
from os.path import exists
import sys
import BugPath
import RevInstances
from CvPythonExtensions import *

#globals
gc = CyGlobalContext()

#szFilename = "OOSLog.txt"
#iMaxFilenameTries = 100

bWroteLog = -1
bWriteEachTurn = False

SEPERATOR = "-----------------------------------------------------------------\n"


# Simply checks every game turn for OOS. If it finds it, writes the
# info contained in the sync checksum to a log file, then sets the bWroteLog
# variable so that it only happens once.
def doGameUpdate():
    global bWroteLog
    bOOS = CyInterface().isOOSVisible()

    #Stop AIAutoPlay
    if (bOOS and CyGame().getAIAutoPlay() > 0):
        RevInstances.AIAutoPlayInst.checkPlayer()
        CyGame().setAIAutoPlay(0)

    if (bWriteEachTurn):
        writeLog()
    else:
        if (bOOS and bWroteLog != gc.getGame().getGameTurn()):
            bWroteLog = gc.getGame().getGameTurn()
            writeLog()

def writeLog():
    PlayerID = 0
    for iPlayer in range(gc.getMAX_PLAYERS()):
        if CyGame().getActivePlayer()==iPlayer:
            PlayerID=iPlayer

    if sys.platform == 'darwin':
        if gc.getDefineINT("ENABLE_OOSLOG") > 1:
            szFilename = BugPath.userDir + "//Beyond the Sword" + "//Logs//" + "OOSLog (Turn " + str(bWroteLog) + ").txt"
        else:
            szFilename = BugPath.userDir + "//Beyond the Sword" + "//Logs//" + "OOSLog (Turn " + str(bWroteLog) + ") Player " + str(PlayerID) + ".txt"
    else:
        if gc.getDefineINT("ENABLE_OOSLOG") > 1:
            szFilename = BugPath.userDir + "\\Beyond the Sword" + "\\Logs\\" + "OOSLog (Turn " + str(bWroteLog) + ").txt"
        else:
            szFilename = BugPath.userDir + "\\Beyond the Sword" + "\\Logs\\" + "OOSLog (Turn " + str(bWroteLog) + ") Player " + str(PlayerID) + ".txt"

    if exists(szFilename):
        return

    pFile = codecs.open(szFilename, "w","utf-8")

    #
    # Global data
    #
    pFile.write(SEPERATOR)
    pFile.write(SEPERATOR)

    pFile.write("  GLOBALS  \n")

    pFile.write(SEPERATOR)
    pFile.write(SEPERATOR)
    pFile.write("\n\n")

    pFile.write("Next Map Rand Value: %d\n" % CyGame().getMapRand().get(10000, "OOS Log"))
    pFile.write("Next Soren Rand Value: %d\n" % CyGame().getSorenRand().get(10000, "OOS Log"))

    pFile.write("Total num cities: %d\n" % CyGame().getNumCities() )
    pFile.write("Total population: %d\n" % CyGame().getTotalPopulation() )
    pFile.write("Total Deals: %d\n" % CyGame().getNumDeals() )

    pFile.write("Total owned plots: %d\n" % CyMap().getOwnedPlots() )
    pFile.write("Total num areas: %d\n" % CyMap().getNumAreas() )

    pFile.write("\n\n")

    #
    # Player data
    #
    for iPlayer in range(gc.getMAX_PLAYERS()):
        pPlayer = gc.getPlayer(iPlayer)
        iTeam = pPlayer.getTeam()
        if (pPlayer.isEverAlive()):
            pFile.write(SEPERATOR)
            pFile.write(SEPERATOR)

            pFile.write("  PLAYER %d,%d \n" % (iPlayer,iTeam))

            pFile.write(SEPERATOR)
            pFile.write(SEPERATOR)
            pFile.write("\n\n")

            pFile.write("Basic data:\n")
            pFile.write("-----------\n")
            pFile.write("Player %d Score: %d\n" % (iPlayer, gc.getGame().getPlayerScore(iPlayer) ))

            pFile.write("Player %d Population: %d\n" % (iPlayer, pPlayer.getTotalPopulation() ) )
            pFile.write("Player %d Total Land: %d\n" % (iPlayer, pPlayer.getTotalLand() ) )
            pFile.write("Player %d Gold: %d\n" % (iPlayer, pPlayer.getGold() ) )
            pFile.write("Player %d Assets: %d\n" % (iPlayer, pPlayer.getAssets() ) )
            pFile.write("Player %d Power: %d\n" % (iPlayer, pPlayer.getPower() ) )
            pFile.write("Player %d Num Cities: %d\n" % (iPlayer, pPlayer.getNumCities() ) )
            pFile.write("Player %d Num Units: %d\n" % (iPlayer, pPlayer.getNumUnits() ) )
            pFile.write("Player %d Num Selection Groups: %d\n" % (iPlayer, pPlayer.getNumSelectionGroups() ) )

            pFile.write("\n\n")

            pFile.write("Player %d Disabled SpellCasting turns: %d\n" % (iPlayer, pPlayer.getDisableSpellcasting() ) )
            pFile.write("Player %d Disabled Production turns: %d\n" % (iPlayer, pPlayer.getDisableProduction() ) )
            pFile.write("Player %d Disabled Research turns: %d\n" % (iPlayer, pPlayer.getDisableResearch() ) )

            pFile.write("\n\n")

            pFile.write("Yields:\n")
            pFile.write("-------\n")
            for iYield in range( int(YieldTypes.NUM_YIELD_TYPES) ):
                pFile.write("Player %d %s Total Yield: %d\n" % (iPlayer, gc.getYieldInfo(iYield).getDescription(), pPlayer.calculateTotalYield(iYield) ))

            pFile.write("\n\n")

            pFile.write("Commerce:\n")
            pFile.write("---------\n")
            for iCommerce in range( int(CommerceTypes.NUM_COMMERCE_TYPES) ):
                pFile.write("Player %d %s Total Commerce: %d\n" % (iPlayer, gc.getCommerceInfo(iCommerce).getDescription(), pPlayer.getCommerceRate(CommerceTypes(iCommerce)) ))

            pFile.write("\n\n")

            pFile.write("Bonus Info:\n")
            pFile.write("-----------\n")
            for iBonus in range(gc.getNumBonusInfos()):
                pFile.write("Player %d, %s, Number Available: %d\n" % (iPlayer, gc.getBonusInfo(iBonus).getDescription(), pPlayer.getNumAvailableBonuses(iBonus) ))
                pFile.write("Player %d, %s, Import: %d\n" % (iPlayer, gc.getBonusInfo(iBonus).getDescription(), pPlayer.getBonusImport(iBonus) ))
                pFile.write("Player %d, %s, Export: %d\n" % (iPlayer, gc.getBonusInfo(iBonus).getDescription(), pPlayer.getBonusExport(iBonus) ))
                pFile.write("\n")

            pFile.write("\n\n")

           # AttributeError: 'CyTeam' object has no attribute 'AI_getWarValue'
           # pFile.write("AIWarValue Info:\n")
           # pFile.write("-----------\n")
           # iTeam = pPlayer.getTeam()
           # eTeam = gc.getTeam(iTeam)
           # for eLoopTeam in range(gc.getMAX_TEAMS()):
           #     if iTeam != eLoopTeam:
           #         pFile.write("Player %d, vs %s, WarValue: %d\n" % (iPlayer, eLoopTeam, eTeam.AI_getWarValue(eLoopTeam)))
           #         pFile.write("\n")

           # pFile.write("\n\n")


            pFile.write("Improvement Info:\n")
            pFile.write("-----------------\n")
            for iImprovement in range(gc.getNumImprovementInfos()):
                pFile.write("Player %d, %s, Improvement count: %d\n" % (iPlayer, gc.getImprovementInfo(iImprovement).getDescription(), pPlayer.getImprovementCount(iImprovement) ))

            pFile.write("\n\n")

            pFile.write("Building Class Info:\n")
            pFile.write("--------------------\n")
            for iBuildingClass in range(gc.getNumBuildingClassInfos()):
                pFile.write("Player %d, %s, Building class count plus building: %d\n" % (iPlayer, gc.getBuildingClassInfo(iBuildingClass).getDescription(), pPlayer.getBuildingClassCountPlusMaking(iBuildingClass) ))

            pFile.write("\n\n")

            pFile.write("Unit Class Info:\n")
            pFile.write("--------------------\n")
            for iUnitClass in range(gc.getNumUnitClassInfos()):
                pFile.write("Player %d, %s, Unit class count plus training: %d\n" % (iPlayer, gc.getUnitClassInfo(iUnitClass).getDescription(), pPlayer.getUnitClassCountPlusMaking(iUnitClass) ))

            pFile.write("\n\n")

            pFile.write("UnitAI Types Info:\n")
            pFile.write("------------------\n")
            for iUnitAIType in range(int(UnitAITypes.NUM_UNITAI_TYPES)):
                pFile.write("Player %d, %s, Unit AI Type count: %d\n" % (iPlayer, gc.getUnitAIInfo(iUnitAIType).getDescription(), pPlayer.AI_totalUnitAIs(UnitAITypes(iUnitAIType)) ))
            

            pFile.write("\n\n")

            pFile.write("Unit Info:\n")
            pFile.write("----------\n")
            iNumUnits = pPlayer.getNumUnits()

            if (iNumUnits == 0):
                pFile.write("No Units")
            else:
                pLoopUnitTuple = pPlayer.firstUnit(False)
                while (pLoopUnitTuple[0] != None):
                    pUnit = pLoopUnitTuple[0]
                    pFile.write("Player %d, Unit ID: %d, %s\n" % (iPlayer, pUnit.getID(), pUnit.getName() ))
                    pFile.write("X: %d, Y: %d\n" % (pUnit.getX(), pUnit.getY()) )
                    pFile.write("GroupID: %d, AIGroupID: %d\n" % (pUnit.getGroupID(), pUnit.getAIGroupID()) )
                    pFile.write("Created: %d\n" % pUnit.getGameTurnCreated() )
                    pFile.write("Power: %d\n" % pUnit.getPower() )
                    pFile.write("Damage: %d\n" % pUnit.getDamage() )
                    pFile.write("Experience: %d\n" % pUnit.getExperience() )
                    pFile.write("Level: %d\n" % pUnit.getLevel() )
                    pFile.write("Moves: %d / %d\n" %(pUnit.movesLeft(),pUnit.maxMoves()))
                    pFile.write("PromotionSpecialization: %d\n" % pUnit.AI_getPromotionSpecialization() )
                    for iPromotion in range(gc.getNumPromotionInfos()):
                        if (pUnit.isHasPromotion(iPromotion)):
                            pFile.write("Promotion: %s\n" % gc.getPromotionInfo(iPromotion).getDescription())

                    pLoopUnitTuple = pPlayer.nextUnit(pLoopUnitTuple[1], False)
                    pFile.write("\n")
                
            pFile.write("\n\n")

            pFile.write("City Info:\n")
            pFile.write("----------\n")
            iNumCities = pPlayer.getNumCities()

            if (iNumCities == 0):
                pFile.write("No Cities")
            else:
                pLoopCityTuple = pPlayer.firstCity(False)
                while (pLoopCityTuple[0] != None):
                    pCity = pLoopCityTuple[0]
                    pFile.write("Player %d, City ID: %d, %s\n" % (iPlayer, pCity.getID(), pCity.getName() ))
                    pFile.write("X: %d, Y: %d\n" % (pCity.getX(), pCity.getY()) )
                    pFile.write("Founded: %d\n" % pCity.getGameTurnFounded() )
                    pFile.write("Acquired: %d\n" % pCity.getGameTurnAcquired() )
                    pFile.write("Populaton: %d\n" % pCity.getPopulation() )
                    pFile.write("Greate People: %d\n" % pCity.getNumGreatPeople() )

#                    for iBuilding in range(gc.getNumBuildingInfos()):
#                        iBuildingClass = gc.getBuildingInfo(iBuilding).getBuildingClassType()
#                        if (pCity.isHasBuilding(iBuilding)):
#                            pFile.write("Building: %s Class: %s\n" % (gc.getBuildingInfo(iBuilding).getDescription(),gc.getBuildingClassInfo(iBuildingClass).getDescription()))
#                            for iBuidlingYield in range( int(YieldTypes.NUM_YIELD_TYPES) ):
#                                pFile.write(" Total Yield %s: %d+%d\n" % (gc.getYieldInfo(iBuidlingYield).getDescription(),gc.getBuildingInfo(iBuilding).getYieldChange(iBuidlingYield),pCity.getBuildingYieldChange(iBuildingClass,iBuidlingYield)) )
#                            for iBuidlingCommerce in range( int(CommerceTypes.NUM_COMMERCE_TYPES) ):
#                                pFile.write(" Total Commerce %s: %d+%d\n" % (gc.getCommerceInfo(iBuidlingCommerce).getDescription(),gc.getBuildingInfo(iBuilding).getCommerceChange(iBuidlingYield),pCity.getBuildingCommerceChange(iBuildingClass,iBuidlingCommerce)) )



                    #for iBuilding in range(gc.getNumBuildingClassInfos()):
                    #    if (pCity.isHasBuildingClass(iBuilding)):
                    #        pFile.write("Building Class: %s\n" % gc.getBuildingClassInfo(iBuilding).getDescription())
                    #        for iBuidlingYield in range( int(YieldTypes.NUM_YIELD_TYPES) ):
                    #            pFile.write("Total Yield %s: %d\n" % (gc.getYieldInfo(iBuidlingYield).getDescription(),pCity.getBuildingYieldChange(iBuilding, iBuidlingYield)) )

                    pLoopCityTuple = pPlayer.nextCity(pLoopCityTuple[1], False)
                    pFile.write("\n")


            pFile.write("Map visibility Info [. = not visible, O owned, x owned by other player, _ not owned by anyone:\n")
            for yPlot in range(CyMap().getGridHeight()):
                for xPlot in range(CyMap().getGridWidth()):
                    pPlot = CyMap().plot(xPlot,yPlot)
                    if not pPlot.isVisible(iTeam,false):
                        pFile.write(". ")
                    else:
                        if pPlot.getOwner() == iPlayer:
                            pFile.write("O ")
                        else:
                            if pPlot.getOwner() == -1:
                                pFile.write("_ ")
                            else:
                                pFile.write("x ")

                pFile.write("\n")


            # Space at end of player's info
            pFile.write("\n\n")
        
    # Close file

    pFile.close()
