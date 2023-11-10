#pragma once

#ifndef CVDEFINES_H
#define CVDEFINES_H

// defines.h

// The following #defines should not be moddable...

#define MOVE_IGNORE_DANGER										(0x00000001)
#define MOVE_SAFE_TERRITORY										(0x00000002)
#define MOVE_NO_ENEMY_TERRITORY								(0x00000004)
#define MOVE_DECLARE_WAR											(0x00000008)
#define MOVE_DIRECT_ATTACK										(0x00000010)
#define MOVE_THROUGH_ENEMY										(0x00000020)
#define MOVE_MAX_MOVES											(0x00000040)

/*************************************************************************************************/
/**	BETTER AI (New Functions Definition) Sephi                                 					**/
/**																								**/
/**						                                            							**/
/*************************************************************************************************/
#define GROUPFLAG_NONE													(1)
#define GROUPFLAG_PERMDEFENSE											(2)
#define GROUPFLAG_PERMDEFENSE_NEW										(3)
#define GROUPFLAG_PATROL    											(4)
#define GROUPFLAG_PATROL_NEW    										(5)
#define GROUPFLAG_EXPLORE                                               (6)
#define GROUPFLAG_DEFENSE_NEW    										(7)
#define GROUPFLAG_DEFENSE       										(8)
#define GROUPFLAG_HERO             										(9)
#define GROUPFLAG_CONQUEST												(10)
#define GROUPFLAG_SETTLERGROUP											(11)
#define GROUPFLAG_HNGROUP                                               (12)
#define GROUPFLAG_PICKUP_EQUIPMENT                                      (13)
#define GROUPFLAG_FEASTING                                              (14)
#define GROUPFLAG_INQUISITION                                           (15)
#define GROUPFLAG_PILLAGE                                               (16)

#define GROUPFLAG_NAVAL_SETTLE                                          (20)
#define GROUPFLAG_NAVAL_SETTLE_NEW                                      (21)
#define GROUPFLAG_NAVAL_SETTLE_PICKUP                                   (22)
#define GROUPFLAG_NAVAL_SETTLE_REINFORCEMENTS                           (23)
#define GROUPFLAG_NAVAL_INVASION                                        (24)
#define GROUPFLAG_NAVAL_INVASION_NEW                                    (25)

#define GROUPFLAG_ANIMAL_CONSTRUCTBUILDING                              (40)
#define GROUPFLAG_SVARTALFAR_KIDNAP										(41)
#define GROUPFLAG_ESUS_MISSIONARY										(42)
#define GROUPFLAG_SUICIDE_SUMMON                                        (43)
#define GROUPFLAG_AWAKENED                                              (44)
#define GROUPFLAG_CREEPER                                               (45)

#define EQUIPMENT_POPUP_SHIFT											(10000) //Used for Equipment to add another Information layer to int

#define NUM_THREAD_OBJECTS												(1)														
/*************************************************************************************************/
/**	END	                                        												**/
/*************************************************************************************************/
/*************************************************************************************************/
/**	MultiBarb                           Sephi                                  					**/
/*************************************************************************************************/
#define GROUPFLAG_MANADEFENDER_DEF										(100)
#define GROUPFLAG_MANADEFENDER_OFF										(101)

#define GROUPFLAG_MERCENARY_HEADHUNTER									(110)
#define GROUPFLAG_MERCENARY_RAIDER										(111)
#define GROUPFLAG_MERCENARY_PILLAGER									(112)
/*************************************************************************************************/
/**	END	                                        												**/
/*************************************************************************************************/

#define RANDPLOT_LAND													(0x00000001)
#define RANDPLOT_UNOWNED											(0x00000002)
#define RANDPLOT_ADJACENT_UNOWNED							(0x00000004)
#define RANDPLOT_ADJACENT_LAND								(0x00000008)
#define RANDPLOT_PASSIBLE											(0x00000010)
#define RANDPLOT_NOT_VISIBLE_TO_CIV						(0x00000020)
#define RANDPLOT_NOT_CITY											(0x00000040)

/*************************************************************************************************/
/**	MultiBarb	merged Sephi			12/23/08									Xienwolf	**/
/**																								**/
/**							Adds extra Barbarian Civilizations									**/
/*************************************************************************************************/
/**								---- Start Original Code ----									**
#ifdef _USRDLL

//FfH: Modified by Kael 09/27/2007
//#define MAX_CIV_PLAYERS												(18)
#define MAX_CIV_PLAYERS												(35)
//FfH: End Modify

#else
#define MAX_CIV_PLAYERS												(CvGlobals::getInstance().getMaxCivPlayers())
#endif

#define MAX_CIV_TEAMS													(MAX_CIV_PLAYERS)
#define MAX_PLAYERS														(MAX_CIV_PLAYERS + 1)
#define MAX_TEAMS															(MAX_PLAYERS)
#define BARBARIAN_PLAYER											((PlayerTypes)MAX_CIV_PLAYERS)
#define BARBARIAN_TEAM												((TeamTypes)MAX_CIV_TEAMS)
/**								----  End Original Code  ----									**/

#ifdef _USRDLL
#define MAX_CIV_PLAYERS											(50)
#else
#define MAX_CIV_PLAYERS											(CvGlobals::getInstance().getMaxCivPlayers())
#endif

#define MAX_CIV_TEAMS											(MAX_CIV_PLAYERS)
#define MAX_PLAYERS												(MAX_CIV_PLAYERS + 8)
#define MAX_TEAMS												(MAX_PLAYERS)
#define BARBARIAN_PLAYER										((PlayerTypes)(MAX_CIV_PLAYERS))
#define BARBARIAN_TEAM											((TeamTypes)(MAX_CIV_TEAMS))
#define ANIMAL_PLAYER										    ((PlayerTypes)(MAX_CIV_PLAYERS + 1))
#define ANIMAL_TEAM											    ((TeamTypes)(MAX_CIV_TEAMS + 1))
#define WILDMANA_PLAYER		    								((PlayerTypes)(MAX_CIV_PLAYERS + 2))
#define WILDMANA_TEAM											((TeamTypes)(MAX_CIV_TEAMS + 2))
#define PIRATES_PLAYER		    								((PlayerTypes)(MAX_CIV_PLAYERS + 3))
#define PIRATES_TEAM											((TeamTypes)(MAX_CIV_TEAMS + 3))
#define WHITEHAND_PLAYER		    							((PlayerTypes)(MAX_CIV_PLAYERS + 4))
#define WHITEHAND_TEAM											((TeamTypes)(MAX_CIV_TEAMS + 4))
#define DEVIL_PLAYER		    								((PlayerTypes)(MAX_CIV_PLAYERS + 5))
#define DEVIL_TEAM												((TeamTypes)(MAX_CIV_TEAMS + 5))
#define ORC_PLAYER		    									((PlayerTypes)(MAX_CIV_PLAYERS + 6))
#define ORC_TEAM												((TeamTypes)(MAX_CIV_TEAMS + 6))
#define MERCENARIES_PLAYER		    							((PlayerTypes)(MAX_CIV_PLAYERS + 7))
#define MERCENARIES_TEAM										((TeamTypes)(MAX_CIV_TEAMS + 7))
#define NUM_BARBARIAN_PLAYERS									MERCENARIES_PLAYER + 1
/**	ADDON (Adventure) Sephi                                                    					**/
#define MAX_ADVENTURESTEPS										(5)

/*************************************************************************************************/
/**	MultiBarb								END													**/
/*************************************************************************************************/

// Char Count limit for edit boxes
#define PREFERRED_EDIT_CHAR_COUNT							(15)
#define MAX_GAMENAME_CHAR_COUNT								(32)
#define MAX_PLAYERINFO_CHAR_COUNT							(32)
#define MAX_PLAYEREMAIL_CHAR_COUNT						(64)
#define MAX_PASSWORD_CHAR_COUNT								(32)
#define MAX_GSLOGIN_CHAR_COUNT								(17)
#define MAX_GSEMAIL_CHAR_COUNT								(50)
#define MAX_GSPASSWORD_CHAR_COUNT							(30)
#define MAX_CHAT_CHAR_COUNT										(256)
#define MAX_ADDRESS_CHAR_COUNT								(64)

#define INVALID_PLOT_COORD										(-(MAX_INT))	// don't use -1 since that is a valid wrap coordinate
#define DIRECTION_RADIUS											(1)
#define DIRECTION_DIAMETER										((DIRECTION_RADIUS * 2) + 1)

//FfH: Modified by Kael 11/18/2007
//#define NUM_CITY_PLOTS												(21)
//#define CITY_HOME_PLOT												(0)
//#define CITY_PLOTS_RADIUS											(2)
#define NUM_CITY_PLOTS												(37)
#define NUM_CITY_PLOTS_STANDARD										(21)
#define CITY_HOME_PLOT												(0)
#define CITY_PLOTS_RADIUS											(3)
//FfH: End Modify

#define CITY_PLOTS_DIAMETER										((CITY_PLOTS_RADIUS*2) + 1)

#define GAME_NAME															("Game")

#define LANDSCAPE_FOW_RESOLUTION							(4)

#define Z_ORDER_LAYER													(-0.1f)
#define Z_ORDER_LEVEL													(-0.3f)

#define CIV4_GUID															"civ4bts"
#define CIV4_PRODUCT_ID												11081
#define CIV4_NAMESPACE_ID											17
#define CIV4_NAMESPACE_EXT										"-tk"

#define MAP_TRANSFER_EXT											"_t"

#define USER_CHANNEL_PREFIX										"#civ4buser!"

#define SETCOLR																L"<color=%d,%d,%d,%d>"
#define ENDCOLR																L"</color>"
#define NEWLINE																L"\n"
#define SEPARATOR															L"\n-----------------------"
#define TEXT_COLOR(szColor)										((int)(GC.getColorInfo((ColorTypes)GC.getInfoTypeForString(szColor)).getColor().r * 255)), ((int)(GC.getColorInfo((ColorTypes)GC.getInfoTypeForString(szColor)).getColor().g * 255)), ((int)(GC.getColorInfo((ColorTypes)GC.getInfoTypeForString(szColor)).getColor().b * 255)), ((int)(GC.getColorInfo((ColorTypes)GC.getInfoTypeForString(szColor)).getColor().a * 255))

// Version Verification files and folders
#ifdef _DEBUG
#define CIV4_EXE_FILE													".\\Civ4BeyondSword_DEBUG.exe"
#define CIV4_DLL_FILE													".\\Assets\\CvGameCoreDLL_DEBUG.dll"
#else
#define CIV4_EXE_FILE													".\\Civ4BeyondSword.exe"
#define CIV4_DLL_FILE													".\\Assets\\CvGameCoreDLL.dll"
#endif
#define CIV4_SHADERS													".\\Shaders\\FXO"
#define CIV4_ASSETS_PYTHON										".\\Assets\\Python"
#define CIV4_ASSETS_XML												".\\Assets\\XML"

#define MAX_PLAYER_NAME_LEN										(64)
#define MAX_VOTE_CHOICES											(8)
#define VOTE_TIMEOUT													(600000)	// 10 minute vote timeout - temporary

#define ANIMATION_DEFAULT											(1)			// Default idle animation

// python module names
#define PYDebugToolModule			"CvDebugInterface"
#define PYScreensModule				"CvScreensInterface"
#define PYCivModule						"CvAppInterface"
#define PYWorldBuilderModule	"CvWBInterface"
#define PYPopupModule					"CvPopupInterface"
#define PYDiplomacyModule			"CvDiplomacyInterface"
#define PYUnitControlModule		"CvUnitControlInterface"
#define PYTextMgrModule				"CvTextMgrInterface"
#define PYPerfTestModule			"CvPerfTest"
#define PYDebugScriptsModule	"DebugScripts"
#define PYPitBossModule				"PbMain"
#define PYTranslatorModule		"CvTranslator"
#define PYGameModule					"CvGameInterface"
#define PYEventModule					"CvEventInterface"
#define PYRandomEventModule					"CvRandomEventInterface"

//FfH: Added by Kael 07/23/2007 (PYSomniumModule and PyDataStorageModule are for Somnium)
#define PYSpellModule					"CvSpellInterface"
#define PYSomniumModule					"CvSomniumInterface"
#define PYDataStorageModule					"CvDataStorageInterface"
//FfH: End Add
#define PYDungeonModule					"CvDungeonInterface"
#define PYGEModule						"CvGlobalEnchantmentInterface"

/*************************************************************************************************/
/**	SPEEDTWEAK (CAR MOD) merged Sephi                                         					**/
/**																								**/
/**	                                                                 							**/
/*************************************************************************************************/
#define DANGER_RANGE			(4)
/*************************************************************************************************/
/**	END	                                        												**/
/*************************************************************************************************/

#endif	// CVDEFINES_H
