#pragma once

#ifndef CVENUMS_h
#define CVENUMS_h

// enums.h

#include "CvDefines.h"

enum GameStateTypes					// Exposed to Python
{
	GAMESTATE_ON,
	GAMESTATE_OVER,
	GAMESTATE_EXTENDED,
};

enum PopupStates						// Exposed to Python
{
	POPUPSTATE_IMMEDIATE,
	POPUPSTATE_QUEUED,
	POPUPSTATE_MINIMIZED,
};

enum PopupEventTypes
{
	POPUPEVENT_NONE,
	POPUPEVENT_PRODUCTION,
	POPUPEVENT_TECHNOLOGY,
	POPUPEVENT_RELIGION,
	POPUPEVENT_WARNING,
	POPUPEVENT_CIVIC,
};

enum CameraLookAtTypes			// Exposed to Python
{
	CAMERALOOKAT_NORMAL,
	CAMERALOOKAT_CITY_ZOOM_IN,
	CAMERALOOKAT_BATTLE,
	CAMERALOOKAT_BATTLE_ZOOM_IN,
	CAMERALOOKAT_SHRINE_ZOOM_IN,
	CAMERALOOKAT_IMMEDIATE,
	CAMERALOOKAT_HOTSEAT,
};

enum CameraMovementSpeeds		// Exposed to Python
{
	CAMERAMOVEMENTSPEED_NORMAL,
	CAMERAMOVEMENTSPEED_SLOW,
	CAMERAMOVEMENTSPEED_FAST,
};

enum CameraAnimationTypes
{
	NO_CAMERA_ANIMATION = -1,
};

enum ZoomLevelTypes					// Exposed to Python
{
	ZOOM_UNKNOWN							= 0x00000000,
	ZOOM_DETAIL								= 0x00000001,
	ZOOM_NORMAL								= 0x00000002,
	ZOOM_GLOBEVIEW_TRANSITION	= 0x00000004,
	ZOOM_GLOBEVIEW						= 0x00000008
};

enum DirectionTypes					// Exposed to Python
{
	NO_DIRECTION = -1,

	DIRECTION_NORTH,
	DIRECTION_NORTHEAST,
	DIRECTION_EAST,
	DIRECTION_SOUTHEAST,
	DIRECTION_SOUTH,
	DIRECTION_SOUTHWEST,
	DIRECTION_WEST,
	DIRECTION_NORTHWEST,

#ifdef _USRDLL
	NUM_DIRECTION_TYPES,
#endif

	DIRECTION_NORTH_MASK = 1 << DIRECTION_NORTH,
	DIRECTION_NORTHEAST_MASK = 1 << DIRECTION_NORTHEAST,
	DIRECTION_EAST_MASK = 1 << DIRECTION_EAST,
	DIRECTION_SOUTHEAST_MASK = 1 << DIRECTION_SOUTHEAST,
	DIRECTION_SOUTH_MASK = 1 << DIRECTION_SOUTH,
	DIRECTION_SOUTHWEST_MASK = 1 << DIRECTION_SOUTHWEST,
	DIRECTION_WEST_MASK = 1 << DIRECTION_WEST,
	DIRECTION_NORTHWEST_MASK = 1 << DIRECTION_NORTHWEST,
};

enum CardinalDirectionTypes			// Exposed to Python
{
	NO_CARDINALDIRECTION = -1,

	CARDINALDIRECTION_NORTH,
	CARDINALDIRECTION_EAST,
	CARDINALDIRECTION_SOUTH,
	CARDINALDIRECTION_WEST,

#ifdef _USRDLL
	NUM_CARDINALDIRECTION_TYPES
#endif
};

enum RotationTypes
{
	ROTATE_NONE = 0,
	ROTATE_90CW,
	ROTATE_180CW,
	ROTATE_270CW,
	NUM_ROTATION_TYPES,

	ROTATE_NONE_MASK	= 1 << ROTATE_NONE,
	ROTATE_90CW_MASK	= 1 << ROTATE_90CW,
	ROTATE_180CW_MASK	= 1 << ROTATE_180CW,
	ROTATE_270CW_MASK	= 1 << ROTATE_270CW,
};

// camera wrap helper
enum WrapDirection
{
	WRAP_SAVE,
	WRAP_NONE = WRAP_SAVE,
	WRAP_RESTORE,
	WRAP_LEFT,
	WRAP_RIGHT,
	WRAP_UP,
	WRAP_DOWN,

#ifdef _USRDLL
	NUM_WRAP_DIRECTIONS,
#endif

	WRAP_LEFT_MASK = 1 << WRAP_LEFT,
	WRAP_RIGHT_MASK = 1 << WRAP_RIGHT,
	WRAP_UP_MASK = 1 << WRAP_UP,
	WRAP_DOWN_MASK = 1 << WRAP_DOWN,
};

enum ColorTypes						// Exposed to Python
{
	NO_COLOR = -1,
};

enum PlayerColorTypes			// Exposed to Python
{
	NO_PLAYERCOLOR = -1,
};

//Warning: these values correspond to locations in the plot texture [JW]
enum PlotStyles						// Exposed to Python
{
	PLOT_STYLE_NONE = -1,

	//first row
	PLOT_STYLE_NUMPAD_1 = 0,
	PLOT_STYLE_NUMPAD_2,
	PLOT_STYLE_NUMPAD_3,
	PLOT_STYLE_NUMPAD_4,
	PLOT_STYLE_NUMPAD_6,
	PLOT_STYLE_NUMPAD_7,
	PLOT_STYLE_NUMPAD_8,
	PLOT_STYLE_NUMPAD_9,

	//second row
	PLOT_STYLE_NUMPAD_1_ANGLED = 8,
	PLOT_STYLE_NUMPAD_2_ANGLED,
	PLOT_STYLE_NUMPAD_3_ANGLED,
	PLOT_STYLE_NUMPAD_4_ANGLED,
	PLOT_STYLE_NUMPAD_6_ANGLED,
	PLOT_STYLE_NUMPAD_7_ANGLED,
	PLOT_STYLE_NUMPAD_8_ANGLED,
	PLOT_STYLE_NUMPAD_9_ANGLED,

	//third row
	PLOT_STYLE_BOX_FILL = 16,
	PLOT_STYLE_BOX_OUTLINE,
	PLOT_STYLE_RIVER_SOUTH,
	PLOT_STYLE_RIVER_EAST,
	PLOT_STYLE_SIDE_ARROWS,
	PLOT_STYLE_CIRCLE,
	PLOT_STYLE_TARGET,
	PLOT_STYLE_DOT_TARGET,

	//fourth row
	PLOT_STYLE_WAVES = 24,
	PLOT_STYLE_DOTS,
	PLOT_STYLE_CIRCLES,
};

//Warning: these values are used as an index into a fixed array
enum PlotLandscapeLayers		// Exposed to Python
{
	PLOT_LANDSCAPE_LAYER_ALL = -1,
	PLOT_LANDSCAPE_LAYER_BASE = 0,
	PLOT_LANDSCAPE_LAYER_RECOMMENDED_PLOTS = 1,
	PLOT_LANDSCAPE_LAYER_WORLD_BUILDER = 2,
	PLOT_LANDSCAPE_LAYER_NUMPAD_HELP = 2,
	PLOT_LANDSCAPE_LAYER_REVEALED_PLOTS = 1,
};

enum AreaBorderLayers
{
	AREA_BORDER_LAYER_REVEALED_PLOTS,
	AREA_BORDER_LAYER_WORLD_BUILDER,
	AREA_BORDER_LAYER_FOUNDING_BORDER,
	AREA_BORDER_LAYER_CITY_RADIUS,
	AREA_BORDER_LAYER_RANGED,
	AREA_BORDER_LAYER_HIGHLIGHT_PLOT,
	AREA_BORDER_LAYER_BLOCKADING,
	AREA_BORDER_LAYER_BLOCKADED,
/*************************************************************************************************/
/**	ADDON(INTERFACEMODE_SPELL_OFFENSIVE) Sephi					                                **/
/*************************************************************************************************/
	AREA_BORDER_LAYER_SPELL_OFFENSIVE,
/*************************************************************************************************/
/**	END                                                                                         **/
/*************************************************************************************************/
	NUM_AREA_BORDER_LAYERS
};

enum EffectTypes
{
	NO_EFFECT = -1,
};

enum AttachableTypes
{
	NO_ATTACHABLE = -1,
};

enum InterfaceModeTypes			// Exposed to Python
{
	NO_INTERFACEMODE = -1,

	INTERFACEMODE_SELECTION,
	INTERFACEMODE_PING,
	INTERFACEMODE_SIGN,
	INTERFACEMODE_GRIP,
	INTERFACEMODE_GLOBELAYER_INPUT,
	INTERFACEMODE_GO_TO,
	INTERFACEMODE_GO_TO_TYPE,
	INTERFACEMODE_GO_TO_ALL,
	INTERFACEMODE_ROUTE_TO,
	INTERFACEMODE_AIRLIFT,
	INTERFACEMODE_NUKE,
	INTERFACEMODE_RECON,
	INTERFACEMODE_PARADROP,
	INTERFACEMODE_AIRBOMB,
	INTERFACEMODE_RANGE_ATTACK,
	INTERFACEMODE_AIRSTRIKE,
	INTERFACEMODE_REBASE,
	INTERFACEMODE_PYTHON_PICK_PLOT,
	INTERFACEMODE_SAVE_PLOT_NIFS,
	INTERFACEMODE_TERRAFORM,
/*************************************************************************************************/
/**	ADDON(INTERFACEMODE_SPELL_OFFENSIVE) Sephi					                                **/
/*************************************************************************************************/
	INTERFACEMODE_SPELL_OFFENSIVE,
/*************************************************************************************************/
/**	END                                                                                         **/
/*************************************************************************************************/
#ifdef _USRDLL
	NUM_INTERFACEMODE_TYPES
#endif
};

enum InterfaceMessageTypes	// Exposed to Python
{
	NO_MESSAGE_TYPE = -1,

	MESSAGE_TYPE_INFO,
	MESSAGE_TYPE_DISPLAY_ONLY,
	MESSAGE_TYPE_MAJOR_EVENT,
	MESSAGE_TYPE_MINOR_EVENT,
	MESSAGE_TYPE_CHAT,
	MESSAGE_TYPE_COMBAT_MESSAGE,
	MESSAGE_TYPE_QUEST,

#ifdef _USRDLL
	NUM_INTERFACE_MESSAGE_TYPES
#endif
};

enum FlyoutTypes
{
	NO_FLYOUT = -1,

	FLYOUT_HURRY,
	FLYOUT_CONSCRIPT,
	FLYOUT_TRAIN,
	FLYOUT_CONSTRUCT,
	FLYOUT_CREATE,
	FLYOUT_MAINTAIN,
	FLYOUT_MOVE_TO,
	FLYOUT_SELECT_UNIT,
	FLYOUT_SELECT_ALL,
	FLYOUT_WAKE_ALL,
	FLYOUR_FORTIFY_ALL,
	FLYOUR_SLEEP_ALL,
};

enum MinimapModeTypes		// Exposed to Python
{
	NO_MINIMAPMODE = -1,

	MINIMAPMODE_TERRITORY,
	MINIMAPMODE_TERRAIN,
	MINIMAPMODE_REPLAY,
	MINIMAPMODE_MILITARY,

#ifdef _USRDLL
	NUM_MINIMAPMODE_TYPES
#endif
};

enum EngineDirtyBits		// Exposed to Python
{
	GlobeTexture_DIRTY_BIT,
	GlobePartialTexture_DIRTY_BIT,
	MinimapTexture_DIRTY_BIT,
	CultureBorders_DIRTY_BIT,

#ifdef _USRDLL
	NUM_ENGINE_DIRTY_BITS
#endif
};

enum InterfaceDirtyBits // Exposed to Python
{
	SelectionCamera_DIRTY_BIT,
	Fog_DIRTY_BIT,
	GlobeLayer_DIRTY_BIT,
	GlobeInfo_DIRTY_BIT,
	Waypoints_DIRTY_BIT,
	PercentButtons_DIRTY_BIT,
	MiscButtons_DIRTY_BIT,
	PlotListButtons_DIRTY_BIT,
	SelectionButtons_DIRTY_BIT,
	CitizenButtons_DIRTY_BIT,
	ResearchButtons_DIRTY_BIT,
	Event_DIRTY_BIT,
	Center_DIRTY_BIT,
	GameData_DIRTY_BIT,
	Score_DIRTY_BIT,
	TurnTimer_DIRTY_BIT,
	Help_DIRTY_BIT,
	MinimapSection_DIRTY_BIT,
	SelectionSound_DIRTY_BIT,
	Cursor_DIRTY_BIT,
	CityInfo_DIRTY_BIT,
	UnitInfo_DIRTY_BIT,
	Popup_DIRTY_BIT,
	CityScreen_DIRTY_BIT,
	InfoPane_DIRTY_BIT,
	Flag_DIRTY_BIT,
	HighlightPlot_DIRTY_BIT,
	ColoredPlots_DIRTY_BIT,
	BlockadedPlots_DIRTY_BIT,
	Financial_Screen_DIRTY_BIT,
	Foreign_Screen_DIRTY_BIT,
	Soundtrack_DIRTY_BIT,
	Domestic_Advisor_DIRTY_BIT,
	Espionage_Advisor_DIRTY_BIT,
	Advanced_Start_DIRTY_BIT,

#ifdef _USRDLL
	NUM_INTERFACE_DIRTY_BITS
#endif
};

enum CityTabTypes			// Exposed to Python
{
	NO_CITYTAB = -1,

	CITYTAB_UNITS,
	CITYTAB_BUILDINGS,
	CITYTAB_WONDERS,

#ifdef _USRDLL
	NUM_CITYTAB_TYPES
#endif
};

enum WidgetTypes					// Exposed to Python
{
	WIDGET_PLOT_LIST,
	WIDGET_PLOT_LIST_SHIFT,
	WIDGET_CITY_SCROLL,
	WIDGET_LIBERATE_CITY,
	WIDGET_CITY_NAME,
	WIDGET_UNIT_NAME,
	WIDGET_CREATE_GROUP,
	WIDGET_DELETE_GROUP,
	WIDGET_TRAIN,
	WIDGET_CONSTRUCT,
	WIDGET_CREATE,
	WIDGET_MAINTAIN,
	WIDGET_HURRY,
	WIDGET_MENU_ICON,
	WIDGET_CONSCRIPT,
	WIDGET_ACTION,
	WIDGET_DISABLED_CITIZEN,
	WIDGET_CITIZEN,
	WIDGET_FREE_CITIZEN,
	WIDGET_ANGRY_CITIZEN,
	WIDGET_CHANGE_SPECIALIST,
	WIDGET_RESEARCH,
	WIDGET_TECH_TREE,
	WIDGET_CHANGE_PERCENT,
	WIDGET_CITY_TAB,
	WIDGET_CONTACT_CIV,
	WIDGET_SCORE_BREAKDOWN,
	WIDGET_ZOOM_CITY,
	WIDGET_END_TURN,
	WIDGET_LAUNCH_VICTORY,
	WIDGET_CONVERT,
	WIDGET_AUTOMATE_CITIZENS,
	WIDGET_AUTOMATE_PRODUCTION,
	WIDGET_EMPHASIZE,
	WIDGET_DIPLOMACY_RESPONSE,
	WIDGET_GENERAL,
	WIDGET_FILE_LISTBOX,
	WIDGET_FILE_EDITBOX,
	WIDGET_WB_UNITNAME_EDITBOX,
	WIDGET_WB_CITYNAME_EDITBOX,
	WIDGET_WB_SAVE_BUTTON,
	WIDGET_WB_LOAD_BUTTON,
	WIDGET_WB_ALL_PLOTS_BUTTON,
	WIDGET_WB_LANDMARK_BUTTON,
	WIDGET_WB_ERASE_BUTTON,
	WIDGET_WB_EXIT_BUTTON,
	WIDGET_WB_UNIT_EDIT_BUTTON,
	WIDGET_WB_CITY_EDIT_BUTTON,
	WIDGET_WB_NORMAL_PLAYER_TAB_MODE_BUTTON,
	WIDGET_WB_NORMAL_MAP_TAB_MODE_BUTTON,
	WIDGET_WB_REVEAL_TAB_MODE_BUTTON,
	WIDGET_WB_DIPLOMACY_MODE_BUTTON,
	WIDGET_WB_REVEAL_ALL_BUTTON,
	WIDGET_WB_UNREVEAL_ALL_BUTTON,
	WIDGET_WB_REGENERATE_MAP,
	WIDGET_TRADE_ITEM,
	WIDGET_UNIT_MODEL,
	WIDGET_FLAG,
	WIDGET_POPUP_QUEUE,

	//	This is meant for python buttons, it will call python functions for display and execution
	WIDGET_PYTHON,

	//	This button type is reserved for widgets meant to be displayed only.  This is meant for general interface text and such...
	WIDGET_HELP_MAINTENANCE,
	WIDGET_HELP_RELIGION,
	WIDGET_HELP_RELIGION_CITY,
	WIDGET_HELP_CORPORATION_CITY,
	WIDGET_HELP_NATIONALITY,
	WIDGET_HELP_DEFENSE,
	WIDGET_HELP_HEALTH,
	WIDGET_HELP_HAPPINESS,
	WIDGET_HELP_POPULATION,
	WIDGET_HELP_PRODUCTION,
	WIDGET_HELP_CULTURE,
	WIDGET_HELP_GREAT_PEOPLE,
	WIDGET_HELP_GREAT_GENERAL,
	WIDGET_HELP_SELECTED,
	WIDGET_HELP_BUILDING,
	WIDGET_HELP_TRADE_ROUTE_CITY,
	WIDGET_HELP_ESPIONAGE_COST,
	WIDGET_HELP_TECH_ENTRY,
	WIDGET_HELP_TECH_PREPREQ,
	WIDGET_HELP_OBSOLETE,
	WIDGET_HELP_OBSOLETE_BONUS,
	WIDGET_HELP_OBSOLETE_SPECIAL,
	WIDGET_HELP_MOVE_BONUS,
	WIDGET_HELP_FREE_UNIT,
	WIDGET_HELP_FEATURE_PRODUCTION,
	WIDGET_HELP_WORKER_RATE,
	WIDGET_HELP_TRADE_ROUTES,
	WIDGET_HELP_HEALTH_RATE,
	WIDGET_HELP_HAPPINESS_RATE,
	WIDGET_HELP_FREE_TECH,
	WIDGET_HELP_LOS_BONUS,
	WIDGET_HELP_MAP_CENTER,
	WIDGET_HELP_MAP_REVEAL,
	WIDGET_HELP_MAP_TRADE,
	WIDGET_HELP_TECH_TRADE,
	WIDGET_HELP_GOLD_TRADE,
	WIDGET_HELP_OPEN_BORDERS,
	WIDGET_HELP_DEFENSIVE_PACT,
	WIDGET_HELP_PERMANENT_ALLIANCE,
	WIDGET_HELP_VASSAL_STATE,
	WIDGET_HELP_BUILD_BRIDGE,
	WIDGET_HELP_IRRIGATION,
	WIDGET_HELP_IGNORE_IRRIGATION,
	WIDGET_HELP_WATER_WORK,
	WIDGET_HELP_IMPROVEMENT,
	WIDGET_HELP_DOMAIN_EXTRA_MOVES,
	WIDGET_HELP_ADJUST,
	WIDGET_HELP_TERRAIN_TRADE,
	WIDGET_HELP_SPECIAL_BUILDING,
	WIDGET_HELP_YIELD_CHANGE,
	WIDGET_HELP_BONUS_REVEAL,
	WIDGET_HELP_CIVIC_REVEAL,
	WIDGET_HELP_PROCESS_INFO,
	WIDGET_HELP_FOUND_RELIGION,
	WIDGET_HELP_FOUND_CORPORATION,
	WIDGET_HELP_FINANCE_NUM_UNITS,
	WIDGET_HELP_FINANCE_UNIT_COST,
	WIDGET_HELP_FINANCE_AWAY_SUPPLY,
	WIDGET_HELP_FINANCE_CITY_MAINT,
	WIDGET_HELP_FINANCE_CIVIC_UPKEEP,
	WIDGET_HELP_FINANCE_FOREIGN_INCOME,
	WIDGET_HELP_FINANCE_INFLATED_COSTS,
	WIDGET_HELP_FINANCE_GROSS_INCOME,
	WIDGET_HELP_FINANCE_NET_GOLD,
	WIDGET_HELP_FINANCE_GOLD_RESERVE,
	WIDGET_HELP_PROMOTION,

	WIDGET_CHOOSE_EVENT,
	WIDGET_PEDIA_JUMP_TO_TECH,
	WIDGET_PEDIA_JUMP_TO_UNIT,
	WIDGET_PEDIA_JUMP_TO_BUILDING,
	WIDGET_PEDIA_JUMP_TO_REQUIRED_TECH,
	WIDGET_PEDIA_JUMP_TO_DERIVED_TECH,
	WIDGET_PEDIA_BACK,
	WIDGET_PEDIA_FORWARD,
	WIDGET_PEDIA_JUMP_TO_BONUS,
	WIDGET_PEDIA_MAIN,
	WIDGET_PEDIA_JUMP_TO_PROMOTION,
	WIDGET_PEDIA_JUMP_TO_UNIT_COMBAT,
	WIDGET_PEDIA_JUMP_TO_IMPROVEMENT,
	WIDGET_PEDIA_JUMP_TO_CIVIC,
	WIDGET_PEDIA_JUMP_TO_CIV,
	WIDGET_PEDIA_JUMP_TO_LEADER,
	WIDGET_PEDIA_JUMP_TO_SPECIALIST,
	WIDGET_PEDIA_JUMP_TO_PROJECT,
	WIDGET_PEDIA_JUMP_TO_TERRAIN,
	WIDGET_PEDIA_JUMP_TO_FEATURE,
	WIDGET_TURN_EVENT,
	WIDGET_FOREIGN_ADVISOR,
	WIDGET_REVOLUTION,
	WIDGET_PEDIA_DESCRIPTION,
	WIDGET_PEDIA_DESCRIPTION_NO_HELP,
	WIDGET_DEAL_KILL,
	WIDGET_MINIMAP_HIGHLIGHT,
	WIDGET_PRODUCTION_MOD_HELP,
	WIDGET_LEADERHEAD,
	WIDGET_LEADER_LINE,
	WIDGET_COMMERCE_MOD_HELP,
	WIDGET_CLOSE_SCREEN,
	WIDGET_PEDIA_JUMP_TO_RELIGION,
	WIDGET_PEDIA_JUMP_TO_CORPORATION,
	WIDGET_GLOBELAYER,
	WIDGET_GLOBELAYER_OPTION,
	WIDGET_GLOBELAYER_TOGGLE,
//>>>>BUGFfH: Added by Denev 2009/09/10
	WIDGET_PEDIA_JUMP_TO_TRAIT,
	WIDGET_HELP_TRAIT,
//<<<<BUGFfH: End Add

//FfH: Added by Kael 07/23/2007
	WIDGET_PEDIA_JUMP_TO_SPELL,
	WIDGET_HELP_SPELL,
//FfH: End Add
/*************************************************************************************************/
/**	ADDON(Screen) Sephi					                                                        **/
/*************************************************************************************************/
	WIDGET_SPELL_RESEARCH,
	WIDGET_GUILD_SCREEN,
	WIDGET_MAGIC_SCREEN,
	WIDGET_DO_TRAIT_FROM_CULTURE,
	WIDGET_DO_SUMMON,
	WIDGET_DO_TERRAFORM,
	WIDGET_DO_GLOBAL_ENCHANTMENT,
	WIDGET_REMOVE_GLOBAL_ENCHANTMENT,
	WIDGET_HELP_MANA,
	WIDGET_HELP_FAITH,
	WIDGET_HELP_LUMBER,
	WIDGET_HELP_LEATHER,
	WIDGET_HELP_METAL,
	WIDGET_HELP_HERB,
	WIDGET_HELP_STONE,
	WIDGET_HELP_GOLD,
	WIDGET_HELP_GLOBAL_CULTURE,
	WIDGET_HELP_CITY_SPECIALIZATION,
	WIDGET_HELP_CIVCOUNTER_ELOHIM,
	WIDGET_HELP_CIVCOUNTER_SCIONS,
	WIDGET_HELP_EQUIPMENT,
	WIDGET_HELP_UNITLIMIT,
	WIDGET_TECH_SCREEN,
	WIDGET_CULTURE_ACHIEVEMENT,
	WIDGET_FOOD_MOD_HELP,
/*************************************************************************************************/
/**	END                                                                                         **/
/*************************************************************************************************/

#ifdef _USRDLL
	NUM_WIDGET_TYPES
#endif
};

enum ButtonPopupTypes			// Exposed to Python
{
	BUTTONPOPUP_TEXT,
	BUTTONPOPUP_MAIN_MENU,
	BUTTONPOPUP_CONFIRM_MENU,
	BUTTONPOPUP_DECLAREWARMOVE,
	BUTTONPOPUP_CONFIRMCOMMAND,
	BUTTONPOPUP_LOADUNIT,
	BUTTONPOPUP_LEADUNIT,
	BUTTONPOPUP_DOESPIONAGE,
	BUTTONPOPUP_DOESPIONAGE_TARGET,
	BUTTONPOPUP_CHOOSETECH,
	BUTTONPOPUP_RAZECITY,
	BUTTONPOPUP_DISBANDCITY,
	BUTTONPOPUP_CHOOSEPRODUCTION,
	BUTTONPOPUP_CHANGECIVIC,
	BUTTONPOPUP_CHANGERELIGION,
	BUTTONPOPUP_CHOOSEELECTION,
	BUTTONPOPUP_DIPLOVOTE,
	BUTTONPOPUP_ALARM,
	BUTTONPOPUP_DEAL_CANCELED,
	BUTTONPOPUP_PYTHON,
	BUTTONPOPUP_PYTHON_SCREEN,
	BUTTONPOPUP_DETAILS,
	BUTTONPOPUP_ADMIN,
	BUTTONPOPUP_ADMIN_PASSWORD,
	BUTTONPOPUP_EXTENDED_GAME,
	BUTTONPOPUP_DIPLOMACY,
	BUTTONPOPUP_ADDBUDDY,
	BUTTONPOPUP_FORCED_DISCONNECT,
	BUTTONPOPUP_PITBOSS_DISCONNECT,
	BUTTONPOPUP_KICKED,
	BUTTONPOPUP_VASSAL_DEMAND_TRIBUTE,
	BUTTONPOPUP_VASSAL_GRANT_TRIBUTE,
	BUTTONPOPUP_EVENT,
	BUTTONPOPUP_FREE_COLONY,
	BUTTONPOPUP_LAUNCH,
	BUTTONPOPUP_FOUND_RELIGION,

//FfH: Added by Kael 11/04/2007
	BUTTONPOPUP_CONFIRMCASTGLOBAL,
	BUTTONPOPUP_CONFIRMCASTWAR,
	BUTTONPOPUP_CONFIRMSETTLEMENT,
//FfH: End Add
/*************************************************************************************************/
/**	ADDON(Screen) Sephi					                                                        **/
/*************************************************************************************************/
	BUTTONPOPUP_NEW_TRAIT,
	BUTTONPOPUP_SUMMON,
	BUTTONPOPUP_TERRAFORM,
	BUTTONPOPUP_GLOBALENCHANTMENT,
	BUTTONPOPUP_GLOBALENCHANTMENT_CANCEL,
	BUTTONPOPUP_PICK_EQUIPMENT,
	BUTTONPOPUP_CHOOSESPELLRESEARCH,
	BUTTONPOPUP_PICK_SACREDKNOWLEDGE,
	BUTTONPOPUP_CITY_INFRASTRUCTURE_INCREASE,
	BUTTONPOPUP_CULTURAL_ACHIEVEMENT,
	BUTTONPOPUP_CHOOSE_GUILD,
	BUTTONPOPUP_CHOOSE_RELIGION,
	BUTTONPOPUP_DUNGEON_EVENT,
/*************************************************************************************************/
/**	END                                                                                         **/
/*************************************************************************************************/

#ifdef _USRDLL
	NUM_BUTTONPOPUP_TYPES
#endif
};

enum ClimateTypes					// Exposed to Python
{
	NO_CLIMATE = -1,
};

enum SeaLevelTypes				// Exposed to Python
{
	NO_SEALEVEL = -1,
};

enum CustomMapOptionTypes	// Exposed to Python
{
	NO_CUSTOM_MAPOPTION = -1,
};

enum WorldSizeTypes				// Exposed to Python
{
	NO_WORLDSIZE = -1,

	WORLDSIZE_DUEL,
	WORLDSIZE_TINY,
	WORLDSIZE_SMALL,
	WORLDSIZE_STANDARD,
	WORLDSIZE_LARGE,
	WORLDSIZE_HUGE,

#ifdef _USRDLL
	NUM_WORLDSIZE_TYPES
#endif
};

// This is our current relationship with each
// one of our connected network peers
enum InitStates
{
	INIT_INACTIVE,
	INIT_CONNECTED,
	INIT_SENT_READY,
	INIT_READY,
	INIT_ASSIGNED_ID,
	INIT_SENT_ID,
	INIT_PEER,
	INIT_FILE_TRANSFER,
	INIT_TRANSFER_COMPLETE,
	INIT_AUTHORIZED,
	INIT_MAP_CONFIRMED,
	INIT_GAME_STARTED,
};

enum TerrainTypes						// Exposed to Python
{
	NO_TERRAIN = -1,
};

enum PlotTypes							// Exposed to Python
{
	NO_PLOT = -1,

	PLOT_PEAK,
	PLOT_HILLS,
	PLOT_LAND,
	PLOT_OCEAN,

#ifdef _USRDLL
	NUM_PLOT_TYPES
#endif
};

enum YieldTypes							// Exposed to Python
{
	NO_YIELD = -1,

	YIELD_FOOD,
	YIELD_PRODUCTION,
	YIELD_COMMERCE,
/*************************************************************************************************/
/**	ADDON (New Yields) Sephi                                                   					**/
/*************************************************************************************************/
	YIELD_LUMBER,
	YIELD_LEATHER,
	YIELD_METAL,
	YIELD_HERB,
	YIELD_STONE,
/*************************************************************************************************/
/**	END	                                        												**/
/*************************************************************************************************/

#ifdef _USRDLL
	NUM_YIELD_TYPES
#endif
};

enum CommerceTypes					// Exposed to Python
{
	NO_COMMERCE = -1,

	COMMERCE_GOLD,
	COMMERCE_RESEARCH,
	COMMERCE_CULTURE,
	COMMERCE_ESPIONAGE,
/*************************************************************************************************/
/**	ADDON (New Mana) Sephi                                                   					**/
/*************************************************************************************************/
	COMMERCE_FAITH,
	COMMERCE_MANA,
	COMMERCE_ARCANE,
/*************************************************************************************************/
/**	END	                                        												**/
/*************************************************************************************************/

#ifdef _USRDLL
	NUM_COMMERCE_TYPES
#endif
};

enum AdvisorTypes						// Exposed to Python
{
	NO_ADVISOR = -1
};

enum FlavorTypes						// Exposed to Python
{
	NO_FLAVOR = -1,

	FLAVOR_MILITARY,
	FLAVOR_RELIGION,
	FLAVOR_PRODUCTION,
	FLAVOR_GOLD,
	FLAVOR_SCIENCE,
	FLAVOR_CULTURE,
	FLAVOR_GROWTH,
	FLAVOR_ESPIONAGE,
	FLAVOR_MELEE,
	FLAVOR_ENGINEERING,
	FLAVOR_RECON,
	FLAVOR_RANGED,
	FLAVOR_MOUNTED,
	FLAVOR_MAGIC,
	FLAVOR_HAPPY,
	FLAVOR_NAVAL,
	FLAVOR_METAL,
	FLAVOR_GUILD,

};

enum EmphasizeTypes					// Exposed to Python
{
	NO_EMPHASIZE = -1,
};

enum GameOptionTypes				// Exposed to Python
{
	NO_GAMEOPTION = -1,
/*************************************************************************************************/
/**	New Gameoption (Definition) Sephi                                         					**/
/**																								**/
/**	                                                                 							**/
/*************************************************************************************************/

	GAMEOPTION_ADVANCED_START,          //hardcoded in the exe to be #0
	GAMEOPTION_AGGRESSIVE_AI,
	GAMEOPTION_CHALLENGE_CUT_LOSERS,
	GAMEOPTION_CHALLENGE_HIGH_TO_LOW,
	GAMEOPTION_CHALLENGE_INCREASING_DIFFICULTY,
	GAMEOPTION_SLOWER_XP,
	GAMEOPTION_ONE_CITY_CHALLENGE,
	GAMEOPTION_LEAD_ANY_CIV,            //hardcoded in the exe to be value 7
	GAMEOPTION_DOUBLE_EVENTS,
	GAMEOPTION_RAGING_BARBARIANS,
	GAMEOPTION_THAW,
	GAMEOPTION_PERMANENT_ALLIANCES,
	GAMEOPTION_FLEXIBLE_DIFFICULTY,
	GAMEOPTION_ALWAYS_WAR,
	GAMEOPTION_COMPLETE_KILLS,
	GAMEOPTION_NEW_RANDOM_SEED,
	GAMEOPTION_NO_CITY_RAZING,
	GAMEOPTION_NO_MAP_TRADING,
	GAMEOPTION_RANDOM_PERSONALITIES,	 //hardcoded in the exe to be value 18
	GAMEOPTION_NO_TECH_BROKERING,
	GAMEOPTION_NO_TECH_TRADING,
	GAMEOPTION_NO_VASSAL_STATES,
	GAMEOPTION_OPTIMIZATION_MEMORY_LARGE_MAPS,
	GAMEOPTION_UNIT_PER_TILE_LIMIT,
	GAMEOPTION_NO_WONDER_UNIQUEUNIT,
/** these are used to make sure, no invisible gameoptions are triggered **/

	GAMEOPTION_DUMMY_02,
	GAMEOPTION_DUMMY_03,
	GAMEOPTION_DUMMY_04,
	GAMEOPTION_DUMMY_05,
	GAMEOPTION_DUMMY_06,
	GAMEOPTION_DUMMY_07,
	GAMEOPTION_DUMMY_08,
	GAMEOPTION_DUMMY_09,

	GAMEOPTION_DUMMY_11,
	GAMEOPTION_DUMMY_12,
	GAMEOPTION_DUMMY_13,
	GAMEOPTION_DUMMY_14,
	GAMEOPTION_DUMMY_15,
	GAMEOPTION_DUMMY_16,
	GAMEOPTION_DUMMY_17,
	GAMEOPTION_DUMMY_18,
	GAMEOPTION_DUMMY_19,

/** END these are used to make sure, no invisible gameoptions are triggered **/
	GAMEOPTION_WB_AGAINST_THE_GREY,
	GAMEOPTION_WB_AGAINST_THE_WALL,
	GAMEOPTION_WB_BARBARIAN_ASSAULT,
	GAMEOPTION_WB_BENEATH_THE_HEEL,
	GAMEOPTION_WB_BLOOD_OF_ANGELS,
	GAMEOPTION_WB_FALL_OF_CUANTINE,
	GAMEOPTION_WB_GIFT_OF_KYLORIN,
	GAMEOPTION_WB_GRAND_MENAGERIE,
	GAMEOPTION_WB_INTO_THE_DESERT,
	GAMEOPTION_WB_LORD_OF_THE_BALORS,
	GAMEOPTION_WB_MULCARN_REBORN,
	GAMEOPTION_WB_RETURN_OF_WINTER,
	GAMEOPTION_WB_THE_BLACK_TOWER,
	GAMEOPTION_WB_THE_CULT,
	GAMEOPTION_WB_THE_MOMUS,
	GAMEOPTION_WB_THE_RADIANT_GUARD,
	GAMEOPTION_WB_THE_SPLINTERED_COURT,
	GAMEOPTION_WB_WAGES_OF_SIN,
	GAMEOPTION_WB_EXTRA,
	GAMEOPTION_WB_LOAD_SCREEN,
	GAMEOPTION_ADVENTURE_MODE,
	GAMEOPTION_NO_PROJECTS,
	GAMEOPTION_NO_HEALING_FOR_HUMANS,
	GAMEOPTION_NO_INFLATION,
	GAMEOPTION_NO_MAINTENANCE,
	GAMEOPTION_NO_WAR_WEARINESS,
	GAMEOPTION_ALWAYS_OPEN_BORDERS,
	GAMEOPTION_ALWAYS_RAZE,
	GAMEOPTION_PICK_RELIGION,

	GAMEOPTION_CHALLENGE_HOLY_CITY,
	GAMEOPTION_NO_BARBARIANS,
	GAMEOPTION_NO_PIRATES,
	GAMEOPTION_NO_CHANGING_WAR_PEACE,
	GAMEOPTION_NO_EVENTS,
	GAMEOPTION_NO_ESPIONAGE,
	GAMEOPTION_NO_LAIRS,
	GAMEOPTION_NO_RELIGION_0,
	GAMEOPTION_NO_RELIGION_1,
	GAMEOPTION_NO_RELIGION_2,
	GAMEOPTION_NO_RELIGION_3,
	GAMEOPTION_NO_RELIGION_4,
	GAMEOPTION_NO_RELIGION_5,
	GAMEOPTION_NO_RELIGION_6,
	GAMEOPTION_AI_NO_BUILDING_PREREQS,
	GAMEOPTION_AI_NO_MINIMUM_LEVEL,
	GAMEOPTION_LOCK_MODS,
	GAMEOPTION_SHOW_HIDDEN_DIPLO_VALUES,
    GAMEOPTION_INFLUENCE_DRIVEN_WAR,
	GAMEOPTION_WINAMP_GUI,
    GAMEOPTION_WILD_MANA,
/*************************************************************************************************/
/**	END	                                        												**/
/*************************************************************************************************/
/**
	GAMEOPTION_ADVANCED_START,

//FfH: Modified by Kael 05/30/2009
//	GAMEOPTION_NO_CITY_RAZING,
//	GAMEOPTION_NO_BARBARIANS,
//	GAMEOPTION_RAGING_BARBARIANS,
//	GAMEOPTION_AGGRESSIVE_AI,
//	GAMEOPTION_LEAD_ANY_CIV,
//	GAMEOPTION_RANDOM_PERSONALITIES,
//	GAMEOPTION_PICK_RELIGION,
//	GAMEOPTION_NO_TECH_TRADING,
//	GAMEOPTION_NO_TECH_BROKERING,
//	GAMEOPTION_PERMANENT_ALLIANCES,
//	GAMEOPTION_ALWAYS_WAR,
//	GAMEOPTION_ONE_CITY_CHALLENGE,
//	GAMEOPTION_NO_CHANGING_WAR_PEACE,
//	GAMEOPTION_NEW_RANDOM_SEED,
//	GAMEOPTION_LOCK_MODS,
//	GAMEOPTION_COMPLETE_KILLS,
//	GAMEOPTION_NO_VASSAL_STATES,
//	GAMEOPTION_NO_EVENTS,
//	GAMEOPTION_NO_ESPIONAGE,
	GAMEOPTION_RAGING_BARBARIANS,
	GAMEOPTION_AGGRESSIVE_AI,
	GAMEOPTION_ONE_CITY_CHALLENGE,
	GAMEOPTION_LEAD_ANY_CIV,
	GAMEOPTION_SLOWER_XP,
	GAMEOPTION_BARBARIAN_WORLD,
	GAMEOPTION_THAW,
	GAMEOPTION_CHALLENGE_CUT_LOSERS,
	GAMEOPTION_CHALLENGE_HIGH_TO_LOW,
	GAMEOPTION_CHALLENGE_INCREASING_DIFFICULTY,
	GAMEOPTION_AI_NO_BUILDING_PREREQS,
	GAMEOPTION_AI_NO_MINIMUM_LEVEL,
	GAMEOPTION_NO_CITY_RAZING,
	GAMEOPTION_NO_BARBARIANS,
	GAMEOPTION_RANDOM_PERSONALITIES,
	GAMEOPTION_PICK_RELIGION,
	GAMEOPTION_NO_TECH_TRADING,
	GAMEOPTION_NO_TECH_BROKERING,
	GAMEOPTION_PERMANENT_ALLIANCES,
	GAMEOPTION_ALWAYS_WAR,
	GAMEOPTION_NO_CHANGING_WAR_PEACE,
	GAMEOPTION_NEW_RANDOM_SEED,
	GAMEOPTION_LOCK_MODS,
	GAMEOPTION_COMPLETE_KILLS,
	GAMEOPTION_NO_VASSAL_STATES,
	GAMEOPTION_NO_EVENTS,
	GAMEOPTION_NO_ESPIONAGE,
	GAMEOPTION_DOUBLE_ANIMALS,
	GAMEOPTION_DOUBLE_BONUSES,
	GAMEOPTION_DOUBLE_EVENTS,
	GAMEOPTION_DOUBLE_GLOBAL_COUNTER,
	GAMEOPTION_NO_LAIRS,
	GAMEOPTION_FLEXIBLE_DIFFICULTY,
	GAMEOPTION_NO_RELIGION_0,
	GAMEOPTION_NO_RELIGION_1,
	GAMEOPTION_NO_RELIGION_2,
	GAMEOPTION_NO_RELIGION_3,
	GAMEOPTION_NO_RELIGION_4,
	GAMEOPTION_NO_RELIGION_5,
	GAMEOPTION_NO_RELIGION_6,
	GAMEOPTION_NO_ORTHUS,
	GAMEOPTION_WB_AGAINST_THE_GREY,
	GAMEOPTION_WB_AGAINST_THE_WALL,
	GAMEOPTION_WB_BARBARIAN_ASSAULT,
	GAMEOPTION_WB_BENEATH_THE_HEEL,
	GAMEOPTION_WB_BLOOD_OF_ANGELS,
	GAMEOPTION_WB_FALL_OF_CUANTINE,
	GAMEOPTION_WB_GIFT_OF_KYLORIN,
	GAMEOPTION_WB_GRAND_MENAGERIE,
	GAMEOPTION_WB_INTO_THE_DESERT,
	GAMEOPTION_WB_LORD_OF_THE_BALORS,
	GAMEOPTION_WB_MULCARN_REBORN,
	GAMEOPTION_WB_RETURN_OF_WINTER,
	GAMEOPTION_WB_THE_BLACK_TOWER,
	GAMEOPTION_WB_THE_CULT,
	GAMEOPTION_WB_THE_MOMUS,
	GAMEOPTION_WB_THE_RADIANT_GUARD,
	GAMEOPTION_WB_THE_SPLINTERED_COURT,
	GAMEOPTION_WB_WAGES_OF_SIN,
	GAMEOPTION_WB_EXTRA,
	GAMEOPTION_WB_LOAD_SCREEN,
	GAMEOPTION_ADVENTURE_MODE,
	GAMEOPTION_NO_PROJECTS,
	GAMEOPTION_NO_HEALING_FOR_HUMANS,
	GAMEOPTION_NO_INFLATION,
	GAMEOPTION_NO_MAINTENANCE,
	GAMEOPTION_NO_WAR_WEARINESS,
	GAMEOPTION_ALWAYS_OPEN_BORDERS,
	GAMEOPTION_ALWAYS_RAZE,
**/
//FfH: End Modify

#ifdef _USRDLL
	NUM_GAMEOPTION_TYPES
#endif
};

enum MultiplayerOptionTypes		// Exposed to Python
{
	NO_MPOPTION = -1,

	MPOPTION_SIMULTANEOUS_TURNS,
	MPOPTION_TAKEOVER_AI,
	MPOPTION_SHUFFLE_TEAMS,
	MPOPTION_ANONYMOUS,
	MPOPTION_TURN_TIMER,

#ifdef _USRDLL
	NUM_MPOPTION_TYPES
#endif
};

enum SpecialOptionTypes			// Exposed to Python
{
	NO_SPECIALOPTION = -1,

	SPECIALOPTION_REPORT_STATS,

#ifdef _USRDLL
	NUM_SPECIALOPTION_TYPES
#endif
};

enum PlayerOptionTypes			// Exposed to Python
{
	NO_PLAYEROPTION = -1,

	PLAYEROPTION_ADVISOR_POPUPS,
	PLAYEROPTION_ADVISOR_HELP,
	PLAYEROPTION_WAIT_END_TURN,
	PLAYEROPTION_MINIMIZE_POP_UPS,
	PLAYEROPTION_SHOW_FRIENDLY_MOVES,
	PLAYEROPTION_SHOW_ENEMY_MOVES,
	PLAYEROPTION_QUICK_MOVES,
	PLAYEROPTION_QUICK_ATTACK,
	PLAYEROPTION_QUICK_DEFENSE,
	PLAYEROPTION_STACK_ATTACK,
	PLAYEROPTION_AUTO_PROMOTION,
	PLAYEROPTION_START_AUTOMATED,
	PLAYEROPTION_SAFE_AUTOMATION,
	PLAYEROPTION_NUMPAD_HELP,
	PLAYEROPTION_NO_UNIT_CYCLING,
	PLAYEROPTION_NO_UNIT_RECOMMENDATIONS,
	PLAYEROPTION_RIGHT_CLICK_MENU,
	PLAYEROPTION_LEAVE_FORESTS,
	PLAYEROPTION_MISSIONARIES_AUTOMATED,
	PLAYEROPTION_MODDER_1,
	PLAYEROPTION_MODDER_2,
	PLAYEROPTION_MODDER_3,

#ifdef _USRDLL
	NUM_PLAYEROPTION_TYPES
#endif
};

enum GraphicOptionTypes			// Exposed to Python
{
	NO_GRAPHICOPTION = -1,

	GRAPHICOPTION_SINGLE_UNIT_GRAPHICS,
	GRAPHICOPTION_HEALTH_BARS,
	GRAPHICOPTION_CITY_DETAIL,
	GRAPHICOPTION_NO_COMBAT_ZOOM,
	GRAPHICOPTION_NO_ENEMY_GLOW,
	GRAPHICOPTION_FROZEN_ANIMATIONS,
	GRAPHICOPTION_EFFECTS_DISABLED,
	GRAPHICOPTION_GLOBE_VIEW_BUILDINGS_DISABLED,
	GRAPHICOPTION_FULLSCREEN,
	GRAPHICOPTION_LOWRES_TEXTURES,
	GRAPHICOPTION_HIRES_TERRAIN,
	GRAPHICOPTION_NO_MOVIES,
	GRAPHICOPTION_CITY_RADIUS,

#ifdef _USRDLL
	NUM_GRAPHICOPTION_TYPES
#endif
};

enum ForceControlTypes			// Exposed to Python
{
	NO_FORCECONTROL = -1,

	FORCECONTROL_SPEED,
	FORCECONTROL_HANDICAP,
	FORCECONTROL_OPTIONS,
	FORCECONTROL_VICTORIES,
	FORCECONTROL_MAX_TURNS,
	FORCECONTROL_MAX_CITY_ELIMINATION,
	FORCECONTROL_ADVANCED_START,

#ifdef _USRDLL
	NUM_FORCECONTROL_TYPES
#endif
};

enum TileArtTypes
{
	TILE_ART_TYPE_NONE = -1,
	TILE_ART_TYPE_TREES,
	TILE_ART_TYPE_HALF_TILING,
	TILE_ART_TYPE_PLOT_TILING,
	NUM_TILE_ART_TYPES
};

enum LightTypes
{
	LIGHT_TYPE_NONE = -1,
	LIGHT_TYPE_SUN,
	LIGHT_TYPE_TERRAIN,
	LIGHT_TYPE_UNIT,
	NUM_LIGHT_TYPES
};

enum VictoryTypes					// Exposed to Python
{
	NO_VICTORY = -1,
};

enum FeatureTypes					// Exposed to Python
{
	NO_FEATURE = -1,
};

enum BonusTypes						// Exposed to Python
{
	NO_BONUS = -1,
};

enum BonusClassTypes			// Exposed to Python
{
	NO_BONUSCLASS = -1,
};

enum ImprovementTypes			// Exposed to Python
{
	NO_IMPROVEMENT = -1,
};

enum RouteTypes						// Exposed to Python
{
	NO_ROUTE = -1,
};

enum RiverTypes						// Exposed to Python
{
	NO_RIVER = -1,
};

enum GoodyTypes						// Exposed to Python
{
	NO_GOODY = -1,
};

enum BuildTypes						// Exposed to Python
{
	NO_BUILD = -1,
};

enum SymbolTypes					// Exposed to Python
{
	NO_SYMBOL = -1,
};

enum FontSymbols					// Exposed to Python
{
	// 'OTHER' symbols
	HAPPY_CHAR = 0,
	UNHAPPY_CHAR,
	HEALTHY_CHAR,
	UNHEALTHY_CHAR,
	BULLET_CHAR,
	STRENGTH_CHAR,
	MOVES_CHAR,
	RELIGION_CHAR,
	STAR_CHAR,
	SILVER_STAR_CHAR,
	TRADE_CHAR,
	DEFENSE_CHAR,
	GREAT_PEOPLE_CHAR,
	BAD_GOLD_CHAR,
	BAD_FOOD_CHAR,
	EATEN_FOOD_CHAR,
	GOLDEN_AGE_CHAR,
	ANGRY_POP_CHAR,
	OPEN_BORDERS_CHAR,
	DEFENSIVE_PACT_CHAR,
	MAP_CHAR,
	OCCUPATION_CHAR,
	POWER_CHAR,
/*************************************************************************************************/
/**	ADDON (Ranged Combat) Sephi								                     				**/
/*************************************************************************************************/
    RANGED_CHAR,
/*************************************************************************************************/
/**	END	                                        												**/
/*************************************************************************************************/

/*************************************************************************************************/
/**	ADDON (New Fontsymbols) Sephi			                                  					**/
/**																								**/
/**						                                            							**/
/*************************************************************************************************/
	CHAR_BARB,
	CHAR_ANIMAL,
	CHAR_DEMON,
	CHAR_ATT_1,
	CHAR_ATT_2,
	CHAR_ATT_3,
	CHAR_ATT_4,
	CHAR_ATT_5,
	CHAR_INFRASTRUCTURE,
/*************************************************************************************************/
/**	END	                                        												**/
/*************************************************************************************************/

#ifdef _USRDLL
	MAX_NUM_SYMBOLS
#endif
};

enum HandicapTypes				// Exposed to Python
{
	NO_HANDICAP = -1,
};

enum GameSpeedTypes				// Exposed to Python
{
	NO_GAMESPEED = -1,
};

enum TurnTimerTypes				// Exposed to Python
{
	NO_TURNTIMER = -1,
};

enum EraTypes							// Exposed to Python
{
	NO_ERA = -1,
};

enum CivilizationTypes		// Exposed to Python
{
	NO_CIVILIZATION = -1,
};

enum LeaderHeadTypes			// Exposed to Python
{
	NO_LEADER = -1,
};

// Used for managing Art Differences based on nationality
enum ArtStyleTypes				// Exposed to Python
{
	NO_ARTSTYLE = -1,
};

enum UnitArtStyleTypes
{
	NO_UNIT_ARTSTYLE = -1,
};

enum CitySizeTypes				// Exposed to Python
{
	NO_CITYSIZE = -1,

	CITYSIZE_SMALL,
	CITYSIZE_MEDIUM,
	CITYSIZE_LARGE,

#ifdef _USRDLL
	NUM_CITYSIZE_TYPES
#endif
};

enum FootstepAudioTypes		// Exposed to Python
{
	NO_FOOTSTEPAUDIO = -1,
};

enum FootstepAudioTags		// Exposed to Python
{
	NO_FOOTSTEPAUDIO_TAG = -1,
};

enum ChatTargetTypes			// Exposed to Python
{
	NO_CHATTARGET = -1,
	CHATTARGET_ALL = -2,
	CHATTARGET_TEAM = -3,
};

enum VoiceTargetTypes			// Exposed to Python
{
	NO_VOICETARGET = -1,

	VOICETARGET_DIPLO,
	VOICETARGET_TEAM,
	VOICETARGET_ALL,

#ifdef _USRDLL
	NUM_VOICETARGETS
#endif
};

enum TeamTypes					// Exposed to Python
{
	NO_TEAM = -1,
};

enum PlayerTypes				// Exposed to Python
{
	NO_PLAYER = -1,
};

enum TraitTypes					// Exposed to Python
{
	NO_TRAIT = -1,
};

enum OrderTypes					// Exposed to Python
{
	NO_ORDER = -1,

	ORDER_TRAIN,
	ORDER_CONSTRUCT,
	ORDER_CREATE,
	ORDER_MAINTAIN,

#ifdef _USRDLL
	NUM_ORDER_TYPES
#endif
};

enum TaskTypes				// Exposed to Python
{
	NO_TASK = -1,

	TASK_RAZE,
	TASK_DISBAND,
	TASK_GIFT,
	TASK_SET_AUTOMATED_CITIZENS,
	TASK_SET_AUTOMATED_PRODUCTION,
	TASK_SET_EMPHASIZE,
	TASK_CHANGE_SPECIALIST,
	TASK_CHANGE_WORKING_PLOT,
	TASK_CLEAR_WORKING_OVERRIDE,
	TASK_HURRY,
	TASK_CONSCRIPT,
	TASK_CLEAR_ORDERS,
	TASK_RALLY_PLOT,
	TASK_CLEAR_RALLY_PLOT,
	TASK_LIBERATE,
/*************************************************************************************************/
/**	ADDON(Screen) Sephi					                                                        **/
/*************************************************************************************************/
	TASK_NEW_TRAIT,
	TASK_SUMMON,
	TASK_TERRAFORM,
	TASK_GLOBAL_ENCHANTMENT,
	TASK_REMOVE_GLOBAL_ENCHANTMENT,
	TASK_CITY_INFRASTRUCTURE_INCREASE,
	TASK_CULTURAL_ACHIEVEMENT,
	TASK_CITY_DUNGEON_EVENT,
/*************************************************************************************************/
/**	END                                                                                         **/
/*************************************************************************************************/

#ifdef _USRDLL
	NUM_TASK_TYPES
#endif
};

enum BuildingClassTypes				// Exposed to Python
{
	NO_BUILDINGCLASS = -1,
};

enum BuildingTypes						// Exposed to Python
{
	NO_BUILDING = -1,
};

enum SpecialBuildingTypes			// Exposed to Python
{
	NO_SPECIALBUILDING = -1,
};

enum ProjectTypes				// Exposed to Python
{
	NO_PROJECT = -1,
};

enum AxisTypes
{
	AXIS_X,
	AXIS_Y,
	AXIS_Z,
	NUM_AXIS_TYPES
};

enum SpaceShipInfoTypes
{
	SPACE_SHIP_INFO_TYPE_NONE,
	SPACE_SHIP_INFO_TYPE_FILENAME,
	SPACE_SHIP_INFO_TYPE_ALPHA_CENTAURI,
	SPACE_SHIP_INFO_TYPE_LAUNCH,
	SPACE_SHIP_INFO_TYPE_LAUNCHED,
	SPACE_SHIP_INFO_TYPE_ZOOM_IN,
	SPACE_SHIP_INFO_TYPE_ZOOM_MOVE,
	SPACE_SHIP_INFO_TYPE_COMPONENT_OFF,
	SPACE_SHIP_INFO_TYPE_COMPONENT_PREVIEW,
	SPACE_SHIP_INFO_TYPE_COMPONENT_APPEAR,
	SPACE_SHIP_INFO_TYPE_COMPONENT_ON,
	SPACE_SHIP_INFO_TYPE_LIGHT_OFF,
	SPACE_SHIP_INFO_TYPE_GANTRY_SMOKE_ON,
	SPACE_SHIP_INFO_TYPE_IN_SPACE_SMOKE_ON,
	SPACE_SHIP_INFO_TYPE_IN_GAME_SMOKE_ON,
	NUM_SPACE_SHIP_INFO_TYPES
};

enum ProcessTypes				// Exposed to Python
{
	NO_PROCESS = -1,
};

enum VoteTypes					// Exposed to Python
{
	NO_VOTE = -1,
};

enum PlayerVoteTypes			// Exposed to Python
{
	NO_PLAYER_VOTE_CHECKED = -6,
	PLAYER_VOTE_NEVER = -5,
	PLAYER_VOTE_ABSTAIN = -4,
	PLAYER_VOTE_NO = -3,
	PLAYER_VOTE_YES = -2,
	NO_PLAYER_VOTE = -1
};

enum InfoBarTypes				// Exposed to Python
{
	INFOBAR_STORED,
	INFOBAR_RATE,
	INFOBAR_RATE_EXTRA,
	INFOBAR_EMPTY,

#ifdef _USRDLL
	NUM_INFOBAR_TYPES
#endif
};

enum HealthBarTypes			// Exposed to Python
{
	HEALTHBAR_ALIVE_ATTACK,
	HEALTHBAR_ALIVE_DEFEND,
	HEALTHBAR_DEAD,

#ifdef _USRDLL
	NUM_HEALTHBAR_TYPES
#endif
};

enum ConceptTypes				// Exposed to Python
{
	NO_CONCEPT = -1,
};

enum NewConceptTypes				// Exposed to Python
{
	NO_NEW_CONCEPT = -1,
};

/*************************************************************************************************/
/**	ADDON (New Pedia Categories) Sephi                                      					**/
/*************************************************************************************************/
enum WildmanaConceptTypes				// Exposed to Python
{
	NO_WILDMANACONCEPT = -1,
};

enum WildmanaGuideTypes				// Exposed to Python
{
	NO_WILDMANAGUIDE = -1,
};
/*************************************************************************************************/
/**	END	                                        												**/
/*************************************************************************************************/

enum CalendarTypes			// Exposed to Python
{
	NO_CALENDAR = -1,

	CALENDAR_DEFAULT,
	CALENDAR_BI_YEARLY,
	CALENDAR_YEARS,
	CALENDAR_TURNS,
	CALENDAR_SEASONS,
	CALENDAR_MONTHS,
	CALENDAR_WEEKS,
};

enum SeasonTypes				// Exposed to Python
{
	NO_SEASON = -1,
};

enum MonthTypes					// Exposed to Python
{
	NO_MONTH = -1,
};

enum DenialTypes				// Exposed to Python
{
	NO_DENIAL = -1,

	DENIAL_UNKNOWN,
	DENIAL_NEVER,
	DENIAL_TOO_MUCH,
	DENIAL_MYSTERY,
	DENIAL_JOKING,
	DENIAL_ANGER_CIVIC,
	DENIAL_FAVORITE_CIVIC,
	DENIAL_MINORITY_RELIGION,
	DENIAL_CONTACT_THEM,
	DENIAL_VICTORY,
	DENIAL_ATTITUDE,
	DENIAL_ATTITUDE_THEM,
	DENIAL_TECH_WHORE,
	DENIAL_TECH_MONOPOLY,
	DENIAL_POWER_US,
	DENIAL_POWER_YOU,
	DENIAL_POWER_THEM,
	DENIAL_TOO_MANY_WARS,
	DENIAL_NO_GAIN,
	DENIAL_NOT_ALLIED,
	DENIAL_RECENT_CANCEL,
	DENIAL_WORST_ENEMY,
	DENIAL_POWER_YOUR_ENEMIES,
	DENIAL_TOO_FAR,
	DENIAL_VASSAL,
	DENIAL_WAR_NOT_POSSIBLE_US,
	DENIAL_WAR_NOT_POSSIBLE_YOU,
	DENIAL_PEACE_NOT_POSSIBLE_US,
	DENIAL_PEACE_NOT_POSSIBLE_YOU
};

enum DomainTypes			// Exposed to Python
{
	NO_DOMAIN = -1,

	DOMAIN_SEA,
	DOMAIN_AIR,
	DOMAIN_LAND,
	DOMAIN_IMMOBILE,

#ifdef _USRDLL
	NUM_DOMAIN_TYPES
#endif
};

enum UnitClassTypes		// Exposed to Python
{
	NO_UNITCLASS = -1,
};

enum UnitTypes				// Exposed to Python
{
	NO_UNIT = -1,
};

enum SpecialUnitTypes	// Exposed to Python
{
	NO_SPECIALUNIT = -1,
};

enum UnitCombatTypes	// Exposed to Python
{
	NO_UNITCOMBAT = -1,
};

enum UnitAITypes			// Exposed to Python
{
	NO_UNITAI = -1,

	UNITAI_UNKNOWN,
	UNITAI_ANIMAL,
	UNITAI_SETTLE,
	UNITAI_WORKER,
	UNITAI_ATTACK,
	UNITAI_ATTACK_CITY,
	UNITAI_COLLATERAL,
	UNITAI_PILLAGE,
	UNITAI_RESERVE,
	UNITAI_COUNTER,
	UNITAI_CITY_DEFENSE,
	UNITAI_CITY_COUNTER,
	UNITAI_CITY_SPECIAL,
	UNITAI_EXPLORE,
	UNITAI_MISSIONARY,
	UNITAI_PROPHET,
	UNITAI_ARTIST,
	UNITAI_SCIENTIST,
	UNITAI_GENERAL,
	UNITAI_MERCHANT,
	UNITAI_ENGINEER,
	UNITAI_SPY,
	UNITAI_ICBM,
	UNITAI_WORKER_SEA,
	UNITAI_ATTACK_SEA,
	UNITAI_RESERVE_SEA,
	UNITAI_ESCORT_SEA,
	UNITAI_EXPLORE_SEA,
	UNITAI_ASSAULT_SEA,
	UNITAI_SETTLER_SEA,
	UNITAI_MISSIONARY_SEA,
	UNITAI_SPY_SEA,
	UNITAI_CARRIER_SEA,
	UNITAI_MISSILE_CARRIER_SEA,
	UNITAI_PIRATE_SEA,
	UNITAI_ATTACK_AIR,
	UNITAI_DEFENSE_AIR,
	UNITAI_CARRIER_AIR,
	UNITAI_MISSILE_AIR,
	UNITAI_PARADROP,
	UNITAI_ATTACK_CITY_LEMMING,
/*************************************************************************************************/
/**	WILDMANA (New Functions Definition) Sephi                                  					**/
/**																								**/
/**						                                            							**/
/*************************************************************************************************/
    UNITAI_MANADEFENDER,
/*************************************************************************************************/
/**	END	                                        												**/
/*************************************************************************************************/
/*************************************************************************************************/
/**	Zusk Mod Fixes                                                          					**/
/**																								**/
/**						                                            							**/
/*************************************************************************************************/
    UNITAI_ACHERON,
    UNITAI_ANIMALDEFENDER,
/*************************************************************************************************/
/**	END	                                        												**/
/*************************************************************************************************/
/*************************************************************************************************/
/**	BETTER AI (New Functions Definition) Sephi                                 					**/
/**																								**/
/**						                                            							**/
/*************************************************************************************************/
    UNITAI_BARBSMASHER,
    UNITAI_HERO,
    UNITAI_FEASTING,
    UNITAI_MEDIC,
	UNITAI_MAGE,
	UNITAI_TERRAFORMER,
	UNITAI_MANA_UPGRADE,
	UNITAI_WARWIZARD,
/*************************************************************************************************/
/**	END	                                        												**/
/*************************************************************************************************/

	NUM_UNITAI_TYPES
};

/*************************************************************************************************/
/**	ADDON (Better AI) Sephi                                      					**/
/*************************************************************************************************/
enum AIGroupTypes			// Exposed to Python
{
	NO_AIGROUP = -1,

	AIGROUP_RESERVE,
	AIGROUP_CITY_DEFENSE,
	AIGROUP_CITY_DEFENSE_REPLACE,
	AIGROUP_SETTLE,
	AIGROUP_WORKER,
	AIGROUP_COUNTER,
	AIGROUP_DESTROY_LAIR,
	AIGROUP_EXPLORE,
	AIGROUP_CITY_INVASION,
	AIGROUP_CITY_INVASION_PILLAGE,
	AIGROUP_SIEGE,
	AIGROUP_MANA_NODE,
	AIGROUP_NAVAL_RESERVE,
	AIGROUP_NAVAL_COUNTER,
	AIGROUP_NAVAL_SETTLE,
	AIGROUP_NAVAL_INVASION,
	AIGROUP_NAVAL_EXPLORER,
	AIGROUP_BARB_PLUNDER,
	AIGROUP_BARB_NEW_LAIR,
	AIGROUP_BARB_GOBLINS,
	AIGROUP_BARB_ORCS,
	AIGROUP_BARB_SKELETONS,
	AIGROUP_BARB_MERCENARIES,
	AIGROUP_BARB_HILLGIANTS,
	AIGROUP_BARB_WEREWOLFS,

	NUM_AIGROUP_TYPES
};

enum CitySpecializationTypes
{
	NO_CITYSPECIALIZATION = -1,

	CITYSPECIALIZATION_GOLD,
	CITYSPECIALIZATION_RESEARCH,
	CITYSPECIALIZATION_CULTURE,
	CITYSPECIALIZATION_ARCANE,
	CITYSPECIALIZATION_MILITARY,
	CITYSPECIALIZATION_METAL,
	CITYSPECIALIZATION_LUMBER,
	CITYSPECIALIZATION_LEATHER,
	CITYSPECIALIZATION_STONE,
	CITYSPECIALIZATION_FAITH,

	NUM_CITYSPECIALIZATION_TYPES
};

enum AIVictoryStrategyTypes
{
	NO_AIVICTORY = -1,

	AIVICTORYSTRATEGY_TOWER_OF_MASTERY,
	AIVICTORYSTRATEGY_CULTURAL,
	AIVICTORYSTRATEGY_RELIGION,
	AIVICTORYSTRATEGY_CONQUEST,

	NUM_AIVICTORYSTRATEGY_TYPES
};

enum UnitCategoryTypes
{
	NO_UNITCATEGORY = -1,
	UNITCATEGORY_SIEGE,
	UNITCATEGORY_WARWIZARD,
	UNITCATEGORY_COMBAT,
	UNITCATEGORY_COMBAT_NAVAL,
	UNITCATEGORY_TRANSPORT_NAVAL,
	UNITCATEGORY_SETTLE,
};

enum AIEconomyTypes
{
	NO_AIECONOMY = -1,
	AIECONOMY_CENTRALIZATION,
	AIECONOMY_COTTAGE,
	AIECONOMY_TRADE,
	AIECONOMY_FOL,
};
/*************************************************************************************************/
/**	END	                                        												**/
/*************************************************************************************************/

enum InvisibleTypes			// Exposed to Python
{
	NO_INVISIBLE = -1,
};

enum VoteSourceTypes			// Exposed to Python
{
	NO_VOTESOURCE = -1,
};

enum ProbabilityTypes		// Exposed to Python
{
	NO_PROBABILITY = -1,

	PROBABILITY_LOW,
	PROBABILITY_REAL,
	PROBABILITY_HIGH,
};

enum ActivityTypes			// Exposed to Python
{
	NO_ACTIVITY = -1,

	ACTIVITY_AWAKE,
	ACTIVITY_HOLD,
	ACTIVITY_SLEEP,
	ACTIVITY_HEAL,
	ACTIVITY_SENTRY,
	ACTIVITY_INTERCEPT,
	ACTIVITY_MISSION,
	ACTIVITY_PATROL,
	ACTIVITY_PLUNDER,

#ifdef _USRDLL
	NUM_ACTIVITY_TYPES
#endif
};

enum AutomateTypes			// Exposed to Python
{
	NO_AUTOMATE = -1,

	AUTOMATE_BUILD,
	AUTOMATE_NETWORK,
	AUTOMATE_CITY,
	AUTOMATE_EXPLORE,
	AUTOMATE_RELIGION,
/*************************************************************************************************/
/**	ADDON (automatic terraforming) Sephi                                     					**/
/**																								**/
/**						                                            							**/
/*************************************************************************************************/
    AUTOMATE_FREE_IMPROVEMENT,
/*************************************************************************************************/
/**	END	                                        												**/
/*************************************************************************************************/
#ifdef _USRDLL
	NUM_AUTOMATE_TYPES
#endif
};

// any additions need to be reflected in GlobalTypes.xml
enum MissionTypes				// Exposed to Python
{
	NO_MISSION = -1,

	MISSION_MOVE_TO,
	MISSION_ROUTE_TO,
	MISSION_MOVE_TO_UNIT,
	MISSION_SKIP,
	MISSION_SLEEP,
	MISSION_FORTIFY,
	MISSION_PLUNDER,
	MISSION_AIRPATROL,
	MISSION_SEAPATROL,
	MISSION_HEAL,
	MISSION_SENTRY,
	MISSION_AIRLIFT,
	MISSION_NUKE,
	MISSION_RECON,
	MISSION_PARADROP,
	MISSION_AIRBOMB,
	MISSION_RANGE_ATTACK,
	MISSION_BOMBARD,
	MISSION_PILLAGE,
	MISSION_SABOTAGE,
	MISSION_DESTROY,
	MISSION_STEAL_PLANS,
	MISSION_FOUND,
	MISSION_SPREAD,
	MISSION_SPREAD_CORPORATION,
	MISSION_JOIN,
	MISSION_CONSTRUCT,
	MISSION_DISCOVER,
	MISSION_HURRY,
	MISSION_TRADE,
	MISSION_GREAT_WORK,
	MISSION_INFILTRATE,
	MISSION_GOLDEN_AGE,
	MISSION_BUILD,
	MISSION_LEAD,
	MISSION_ESPIONAGE,
	MISSION_DIE_ANIMATION,

	MISSION_BEGIN_COMBAT,
	MISSION_END_COMBAT,
	MISSION_AIRSTRIKE,
	MISSION_SURRENDER,
	MISSION_CAPTURED,
	MISSION_IDLE,
	MISSION_DIE,
	MISSION_DAMAGE,
	MISSION_MULTI_SELECT,
	MISSION_MULTI_DESELECT,
/*************************************************************************************************/
/**	ADDON(INTERFACEMODE_SPELL_OFFENSIVE) Sephi					                                **/
/*************************************************************************************************/
	MISSION_SPELL_OFFENSIVE,
	MISSION_TERRAFORM,
/*************************************************************************************************/
/**	END                                                                                         **/
/*************************************************************************************************/
#ifdef _USRDLL
	NUM_MISSION_TYPES
#endif
};

enum MissionAITypes		// Exposed to Python
{
	NO_MISSIONAI = -1,

	MISSIONAI_SHADOW,
	MISSIONAI_GROUP,
	MISSIONAI_LOAD_ASSAULT,
	MISSIONAI_LOAD_SETTLER,
	MISSIONAI_LOAD_SPECIAL,
	MISSIONAI_GUARD_CITY,
	MISSIONAI_GUARD_BONUS,
	MISSIONAI_GUARD_TRADE_NET,
	MISSIONAI_GUARD_SPY,
	MISSIONAI_ATTACK_SPY,
	MISSIONAI_SPREAD,
	MISSIONAI_SPREAD_CORPORATION,
	MISSIONAI_CONSTRUCT,
	MISSIONAI_HURRY,
	MISSIONAI_GREAT_WORK,
	MISSIONAI_EXPLORE,
	MISSIONAI_BLOCKADE,
	MISSIONAI_PILLAGE,
	MISSIONAI_FOUND,
	MISSIONAI_BUILD,
	MISSIONAI_ASSAULT,
	MISSIONAI_CARRIER,
	MISSIONAI_PICKUP
};

// any additions need to be reflected in GlobalTypes.xml
enum CommandTypes					// Exposed to Python
{
	NO_COMMAND = -1,

	COMMAND_PROMOTION,
	COMMAND_UPGRADE,
	COMMAND_AUTOMATE,
/*************************************************************************************************/
/**	ADDON (automatic Spellcasting) Sephi                                     					**/
/**																								**/
/**						                                            							**/
/*************************************************************************************************/
    COMMAND_AUTOMATE_SPELL,
/*************************************************************************************************/
/**	END	                                        												**/
/*************************************************************************************************/
/*************************************************************************************************/
/**	ADDON (Ranged Combat) Sephi								                     				**/
/**																								**/
/**						                                            							**/
/*************************************************************************************************/
    COMMAND_RANGESTRIKE,
/*************************************************************************************************/
/**	END	                                        												**/
/*************************************************************************************************/
	COMMAND_WAKE,
	COMMAND_CANCEL,
	COMMAND_CANCEL_ALL,
	COMMAND_STOP_AUTOMATION,
	COMMAND_DELETE,
	COMMAND_GIFT,
	COMMAND_LOAD,
	COMMAND_LOAD_UNIT,
	COMMAND_UNLOAD,
	COMMAND_UNLOAD_ALL,
	COMMAND_HOTKEY,

//FfH: Added by Kael 07/23/2007
	COMMAND_CAST,
//FfH: End Add
	COMMAND_PROJECT,
	COMMAND_PICK_EQUIPMENT,
	COMMAND_GARRISION,
	COMMAND_MOBILIZE,
	COMMAND_USE_PORTAL,

#ifdef _USRDLL
	NUM_COMMAND_TYPES
#endif
};

enum ControlTypes					// Exposed to Python
{
	NO_CONTROL = -1,

	CONTROL_CENTERONSELECTION,
	CONTROL_SELECTYUNITTYPE,
	CONTROL_SELECTYUNITALL,
	CONTROL_SELECTCITY,
	CONTROL_SELECTCAPITAL,
	CONTROL_NEXTCITY,
	CONTROL_PREVCITY,
	CONTROL_NEXTUNIT,
	CONTROL_PREVUNIT,
	CONTROL_CYCLEUNIT,
	CONTROL_CYCLEUNIT_ALT,
	CONTROL_CYCLEWORKER,
	CONTROL_LASTUNIT,
	CONTROL_ENDTURN,
	CONTROL_ENDTURN_ALT,
	CONTROL_FORCEENDTURN,
	CONTROL_AUTOMOVES,
	CONTROL_PING,
	CONTROL_SIGN,
	CONTROL_GRID,
	CONTROL_BARE_MAP,
	CONTROL_YIELDS,
	CONTROL_RESOURCE_ALL,
	CONTROL_UNIT_ICONS,
	CONTROL_GLOBELAYER,
	CONTROL_SCORES,
	CONTROL_LOAD_GAME,
	CONTROL_OPTIONS_SCREEN,
	CONTROL_RETIRE,
	CONTROL_SAVE_GROUP,
	CONTROL_SAVE_NORMAL,
	CONTROL_QUICK_SAVE,
	CONTROL_QUICK_LOAD,
	CONTROL_ORTHO_CAMERA,
	CONTROL_CYCLE_CAMERA_FLYING_MODES,
	CONTROL_ISOMETRIC_CAMERA_LEFT,
	CONTROL_ISOMETRIC_CAMERA_RIGHT,
	CONTROL_FLYING_CAMERA,
	CONTROL_MOUSE_FLYING_CAMERA,
	CONTROL_TOP_DOWN_CAMERA,
	CONTROL_CIVILOPEDIA,
	CONTROL_RELIGION_SCREEN,
	CONTROL_CORPORATION_SCREEN,
	CONTROL_CIVICS_SCREEN,
	CONTROL_FOREIGN_SCREEN,
	CONTROL_FINANCIAL_SCREEN,
	CONTROL_MILITARY_SCREEN,
	CONTROL_TECH_CHOOSER,
	CONTROL_TURN_LOG,

//FfH: Added by Kael 09/24/2008
	CONTROL_TROPHY,
//FfH: End Add
/*************************************************************************************************/
/**	ADDON(Houses of Erebus/Screen) Sephi                                                        **/
/*************************************************************************************************/
	CONTROL_GUILD_SCREEN,
	CONTROL_MAGIC_SCREEN,
	CONTROL_ARCANE_SCREEN,
/*************************************************************************************************/
/**	END                                                                                         **/
/*************************************************************************************************/
	CONTROL_CHAT_ALL,
	CONTROL_CHAT_TEAM,
	CONTROL_DOMESTIC_SCREEN,
	CONTROL_VICTORY_SCREEN,
	CONTROL_INFO,
	CONTROL_GLOBE_VIEW,
	CONTROL_DETAILS,
	CONTROL_ADMIN_DETAILS,
	CONTROL_HALL_OF_FAME,
	CONTROL_WORLD_BUILDER,
	CONTROL_DIPLOMACY,
	CONTROL_SELECT_HEALTHY,
	CONTROL_ESPIONAGE_SCREEN,
	CONTROL_FREE_COLONY,
/*************************************************************************************************/
/**	ADDON (WINAMP) merged Sephi																	**/
/*************************************************************************************************/
	CONTROL_WINAMP_PLAY,
	CONTROL_WINAMP_STOP,
	CONTROL_WINAMP_PAUSE,
	CONTROL_WINAMP_NEXT,
	CONTROL_WINAMP_PREV,
/*************************************************************************************************/
/**	ADDON (WINAMP)						END														**/
/*************************************************************************************************/


#ifdef _USRDLL
	NUM_CONTROL_TYPES
#endif
};

enum PromotionTypes				// Exposed to Python
{
	NO_PROMOTION = -1,
};

//FfH: Added by Kael 07/23/2007
enum DllExport AlignmentTypes
{
	NO_ALIGNMENT = -1,

    ALIGNMENT_GOOD,
    ALIGNMENT_NEUTRAL,
    ALIGNMENT_EVIL,

#ifdef _USRDLL
	NUM_ALIGNMENT_TYPES
#endif
};

enum DllExport DamageTypes
{
	NO_DAMAGE = -1,

    DAMAGE_COLD,
    DAMAGE_FIRE,
    DAMAGE_HOLY,
    DAMAGE_LIGHTNING,
    DAMAGE_PHYSICAL,
    DAMAGE_POISON,
    DAMAGE_UNHOLY,

#ifdef _USRDLL
	NUM_DAMAGE_TYPES
#endif
};

enum DllExport SpellTypes
{
	NO_SPELL = -1,
};
//FfH: End Add

/*************************************************************************************************/
/**	BETTER AI (Better Promotions) Sephi                                      					**/
/**																								**/
/**						                                            							**/
/*************************************************************************************************/
enum DllExport AIPromotionSpecializationTypes
{
	NO_AIPROMOTIONSPECIALIZATION = -1,
};

/**	ADDON (Adventure) Sephi                                                      					**/
enum DllExport AdventureTypes
{
	NO_ADVENTURE = -1,
};
enum DllExport AdventureStepTypes
{
	NO_ADVENTURESTEP = -1,
};

/**	ADDON (Mana Schools) Sephi                                                      					**/
enum DllExport ManaschoolTypes
{
	NO_MANASCHOOL = -1,
};

/**	ADDON (Combatauras) Sephi																	**/
enum DllExport CombatAuraTypes
{
	NO_COMBATAURA = -1,
};

enum DllExport CombatAuraClassTypes
{
	NO_COMBATAURACLASS = -1,
};

enum DllExport EquipmentCategoryTypes
{
	NO_EQUIPMENTCATEGORY = -1,
};

enum DllExport DurabilityTypes
{
	NO_DURABILITY = -1,
};

enum DllExport DungeonTypes
{
	NO_DUNGEON = -1,
};

enum DllExport DungeonEventTypes
{
	NO_DUNGEONEVENT = -1,
};

enum DllExport LairResultTypes
{
	NO_LAIRRESULT = -1,
};

enum DllExport CityStateTypes
{
	NO_CITYSTATE = -1,
};

enum DllExport PlaneTypes
{
	NO_PLANE = -1,
};

enum DllExport TerrainClassTypes
{
	NO_TERRAINCLASS = -1,
};

enum DllExport FeatureClassTypes
{
	NO_FEATURECLASS = -1,
};
/*************************************************************************************************/
/**	END	                                        												**/
/*************************************************************************************************/

enum TechTypes						// Exposed to Python
{
	NO_TECH = -1,
};

enum SpecialistTypes			// Exposed to Python
{
	NO_SPECIALIST = -1,
};

//Changes here need to be applied to the convertReligionTypes function as well
enum ReligionTypes				// Exposed to Python
{
	NO_RELIGION = -1,
	RELIGION_FELLOWSHIP_OF_LEAVES,
	RELIGION_THE_ORDER,
	RELIGION_OCTOPUS_OVERLORDS,
	RELIGION_RUNES_OF_KILMORPH,
	RELIGION_THE_ASHEN_VEIL,
	RELIGION_THE_EMPYREAN,
	RELIGION_COUNCIL_OF_ESUS,

#ifdef _USRDLL
	NUM_RELIGION_TYPES
#endif
};

enum CorporationTypes				// Exposed to Python
{
	NO_CORPORATION = -1,
};

enum HurryTypes						// Exposed to Python
{
	NO_HURRY = -1,

	HURRY_POPULATION,
	HURRY_GOLD,
	HURRY_STONE,

	NUM_HURRY_TYPES
};

enum UpkeepTypes					// Exposed to Python
{
	NO_UPKEEP = -1,
};

enum CultureLevelTypes		// Exposed to Python
{
	NO_CULTURELEVEL = -1,
};

enum CivicOptionTypes			// Exposed to Python
{
	NO_CIVICOPTION = -1,
};

enum CivicTypes						// Exposed to Python
{
	NO_CIVIC = -1,
};

enum WarPlanTypes					// Exposed to Python
{
	NO_WARPLAN = -1,

	WARPLAN_ATTACKED_RECENT,
	WARPLAN_ATTACKED,
	WARPLAN_PREPARING_LIMITED,
	WARPLAN_PREPARING_TOTAL,
	WARPLAN_LIMITED,
	WARPLAN_TOTAL,
	WARPLAN_DOGPILE
};

enum AreaAITypes					// Exposed to Python
{
	NO_AREAAI = -1,

	AREAAI_OFFENSIVE,
	AREAAI_DEFENSIVE,
	AREAAI_MASSING,
	AREAAI_ASSAULT,
	AREAAI_ASSAULT_MASSING,
	AREAAI_ASSAULT_ASSIST,
	AREAAI_NEUTRAL
};

enum EndTurnButtonStates	// Exposed to Python
{
	END_TURN_GO,
	END_TURN_OVER_HIGHLIGHT,
	END_TURN_OVER_DARK,

#ifdef _USRDLL
	NUM_END_TURN_STATES
#endif
};

enum FogOfWarModeTypes		// Exposed to Python
{
	FOGOFWARMODE_OFF,
	FOGOFWARMODE_UNEXPLORED,
	FOGOFWARMODE_NOVIS,

#ifdef _USRDLL
	NUM_FOGOFWARMODE_TYPES
#endif
};

enum FogTypes
{
	FOG_TYPE_NONE,
	FOG_TYPE_PARALLEL,
	FOG_TYPE_PROJECTED,
	NUM_FOG_TYPES
};

enum CameraOverlayTypes
{
	CAMERA_OVERLAY_DECAL,
	CAMERA_OVERLAY_ADDITIVE,
	NUM_CAMERA_OVERLAY_TYPES
};

enum FOWUpdateTypes
{
	FOW_UPDATE_REGULAR,
	FOW_UPDATE_IMMEDIATE,
	FOW_UPDATE_UNDO_IMMEDIATE,
	FOW_UPDATE_FORCE_CHANGE,
	NUM_FOW_UPDATE_TYPES
};

// AnimationTypes is depreciated, and will be eventually removed.
// BONUSANIMATION_* and IMPROVEMENTANIMATION_* are still used, and will be left.

enum AnimationTypes			// Exposed to Python
{
	NONE_ANIMATION = -1,	// NO_ANIMATION is used by FirePlace

	BONUSANIMATION_UNIMPROVED = 1,
	BONUSANIMATION_NOT_WORKED,
	BONUSANIMATION_WORKED,

	IMPROVEMENTANIMATION_OFF = 2,
	IMPROVEMENTANIMATION_ON,
	IMPROVEMENTANIMATION_OFF_EXTRA,
	IMPROVEMENTANIMATION_ON_EXTRA_1,
	IMPROVEMENTANIMATION_ON_EXTRA_2,
	IMPROVEMENTANIMATION_ON_EXTRA_3,
	IMPROVEMENTANIMATION_ON_EXTRA_4,
};

enum EntityEventTypes		// Exposed to Python
{
	ENTITY_EVENT_NONE = -1,			//!< Invalid event
};

enum AnimationPathTypes	// Exposed to Python
{
	ANIMATIONPATH_NONE = -1,

	// Default animation paths
	ANIMATIONPATH_IDLE,
	ANIMATIONPATH_MOVE,
	ANIMATIONPATH_DAMAGE,				//!< Updates the damage state for the unit

	// Combat related animation paths
	ANIMATIONPATH_RANDOMIZE_ANIMATION_SET,
	ANIMATIONPATH_NUKE_STRIKE,
	ANIMATIONPATH_MELEE_STRIKE,
	ANIMATIONPATH_MELEE_HURT,
	ANIMATIONPATH_MELEE_DIE,
	ANIMATIONPATH_MELEE_FORTIFIED,
	ANIMATIONPATH_MELEE_DIE_FADE,		//!< Used only in combat. The colateral damage die should have a fade integrated.
	ANIMATIONPATH_MELEE_FLEE,			//!< Used only by settler children, so they don't die in combat

	// Ranged combat related animation paths
	ANIMATIONPATH_RANGED_STRIKE,
	ANIMATIONPATH_RANGED_DIE,
	ANIMATIONPATH_RANGED_FORTIFIED,
	ANIMATIONPATH_RANGED_RUNHIT,
	ANIMATIONPATH_RANGED_RUNDIE,
	ANIMATIONPATH_RANGED_DIE_FADE,		//!< Used only in combat. The colateral damage die should have a fade integrated.
	ANIMATIONPATH_LEADER_COMMAND,

	// Air Units animation paths
	ANIMATIONPATH_AIRFADEIN,
	ANIMATIONPATH_AIRFADEOUT,
	ANIMATIONPATH_AIRSTRIKE,
	ANIMATIONPATH_AIRBOMB,

	//mission related animation paths
	ANIMATIONPATH_HEAL,
	ANIMATIONPATH_SLEEP,
	ANIMATIONPATH_FORTIFY,
	ANIMATIONPATH_MELEE_FORTIFY,
	ANIMATIONPATH_PILLAGE,
	ANIMATIONPATH_SENTRY,
	ANIMATIONPATH_FOUND,
	ANIMATIONPATH_IRRIGATE,
	ANIMATIONPATH_BUILD,
	ANIMATIONPATH_MINE,
	ANIMATIONPATH_CHOP,
	ANIMATIONPATH_SHOVEL,
	ANIMATIONPATH_RAILROAD,
	ANIMATIONPATH_SABOTAGE,
	ANIMATIONPATH_DESTROY,
	ANIMATIONPATH_STEAL_PLANS,
	ANIMATIONPATH_GREAT_EVENT,
	ANIMATIONPATH_SURRENDER,
	ANIMATIONPATH_AIRPATROL,
};

//!<  Enumeration for the animation category types.
enum AnimationCategoryTypes			// Exposed to Python
{
	ANIMCAT_NONE = -1,
};

//!< Animation category operators.
enum AnimationOperatorTypes
{
	ANIMOP_FIRST = 1001,
	ANIMOP_RAND_FPR = ANIMOP_FIRST,			//!< Sets the FPR to a random number [0..1].
	ANIMOP_SET_FPR,							//!< Sets the FPR to the input value
	ANIMOP_SET_BR,							//!< Sets the BR to true the input value is non-zero
	ANIMOP_ADD_FPR,							//!< Adds the input value to the FPR register (input can be negative)
	ANIMOP_TEST_GT,							//!< Sets/resets the BR if the FPR is greater than the input
	ANIMOP_TEST_LT,							//!< Sets/resets the BR if the FPR is less than the input
	ANIMOP_LOAD_CAS,						//!< Sets the FPR to the CAS/32.
	ANIMOP_SET_ANIMATION_SET,						//!< Modifies the current animation set, by setting it to ((FPR*32)%32)
	ANIMOP_SKIP_IF,							//!< Skips the given number of instructions if the BR is true
	ANIMOP_CAS_INCR,						//!< Increments the CAS (and resets to zero if it is the maximum)
	ANIMOP_IS_RANGED,						//!< Sets the BR to true if the unit is "ranged", and to false otherwise
	ANIMOP_DEACTIVATE_DAMAGE_LAYERS,					//!< Deactivates all animations that are layered
	ANIMOP_SETCAS_DAMAGE,					//!< Set the current animation set to the damage level of the unit (damage/art define max)
	ANIMOP_LAST
};

enum CursorTypes							// Exposed to Python
{
	NO_CURSOR = -1,
};

enum FunctionTypes
{
	FUNC_NOINTERP = 0,	// NiAnimationKey::NOINTERP,
	FUNC_LINKEY,	//	= NiAnimationKey::LINKEY,
	FUNC_BEZKEY,	//	= NiAnimationKey::BEZKEY,
	FUNC_TCBKEY,	// = NiAnimationKey::TCBKEY,
	FUNC_EULERKEY,	// = NiAnimationKey::EULERKEY,
	FUNC_STEPKEY,	// = NiAnimationKey::STEPKEY,

#ifdef _USRDLL
	NUM_FUNC_TYPES	// = NiAnimationKey::NUMKEYTYPES
#endif
};

enum TradeableItems						// Exposed to Python
{
	TRADE_ITEM_NONE = -1,

	TRADE_GOLD,
	TRADE_GOLD_PER_TURN,
	TRADE_MAPS,
	TRADE_VASSAL,
	TRADE_SURRENDER,
	TRADE_OPEN_BORDERS,
	TRADE_DEFENSIVE_PACT,
	TRADE_PERMANENT_ALLIANCE,
	TRADE_PEACE_TREATY,

#ifdef _USRDLL
	NUM_BASIC_ITEMS,

	TRADE_TECHNOLOGIES = NUM_BASIC_ITEMS,
#else
	TRADE_TECHNOLOGIES,
#endif

	TRADE_RESOURCES,
	TRADE_CITIES,
	TRADE_PEACE,
	TRADE_WAR,
	TRADE_EMBARGO,
	TRADE_CIVIC,
	TRADE_RELIGION,

#ifdef _USRDLL
	NUM_TRADEABLE_HEADINGS,

	NUM_TRADEABLE_ITEMS = NUM_TRADEABLE_HEADINGS,
#endif
};

enum DiploEventTypes					// Exposed to Python
{
	NO_DIPLOEVENT = -1,

	DIPLOEVENT_CONTACT,
	DIPLOEVENT_AI_CONTACT,
	DIPLOEVENT_FAILED_CONTACT,
	DIPLOEVENT_GIVE_HELP,
	DIPLOEVENT_REFUSED_HELP,
	DIPLOEVENT_ACCEPT_DEMAND,
	DIPLOEVENT_REJECTED_DEMAND,
	DIPLOEVENT_DEMAND_WAR,
	DIPLOEVENT_CONVERT,
	DIPLOEVENT_NO_CONVERT,
	DIPLOEVENT_REVOLUTION,
	DIPLOEVENT_NO_REVOLUTION,
	DIPLOEVENT_JOIN_WAR,
	DIPLOEVENT_NO_JOIN_WAR,
	DIPLOEVENT_STOP_TRADING,
	DIPLOEVENT_NO_STOP_TRADING,
	DIPLOEVENT_ASK_HELP,
	DIPLOEVENT_MADE_DEMAND,
	DIPLOEVENT_RESEARCH_TECH,
	DIPLOEVENT_TARGET_CITY,
	DIPLOEVENT_MADE_DEMAND_VASSAL,

#ifdef _USRDLL
	NUM_DIPLOEVENT_TYPES
#endif
};

enum DiploCommentTypes				// Exposed to Python
{
	NO_DIPLOCOMMENT = -1
};

enum NetContactTypes					// Exposed to Python
{
	NO_NETCONTACT = -1,
	NETCONTACT_INITIAL,
	NETCONTACT_RESPONSE,
	NETCONTACT_ESTABLISHED,
	NETCONTACT_BUSY,

#ifdef _USRDLL
	NUM_NETCONTACT_TYPES
#endif
};

enum ContactTypes							// Exposed to Python
{
	CONTACT_RELIGION_PRESSURE,
	CONTACT_CIVIC_PRESSURE,
	CONTACT_JOIN_WAR,
	CONTACT_STOP_TRADING,
	CONTACT_GIVE_HELP,
	CONTACT_ASK_FOR_HELP,
	CONTACT_DEMAND_TRIBUTE,
	CONTACT_OPEN_BORDERS,
	CONTACT_DEFENSIVE_PACT,
	CONTACT_PERMANENT_ALLIANCE,
	CONTACT_PEACE_TREATY,
	CONTACT_TRADE_TECH,
	CONTACT_TRADE_BONUS,
	CONTACT_TRADE_MAP,

	NUM_CONTACT_TYPES
};

enum MemoryTypes							// Exposed to Python
{
	MEMORY_DECLARED_WAR,
	MEMORY_DECLARED_WAR_ON_FRIEND,
	MEMORY_HIRED_WAR_ALLY,
	MEMORY_NUKED_US,
	MEMORY_NUKED_FRIEND,
	MEMORY_RAZED_CITY,
	MEMORY_RAZED_HOLY_CITY,
	MEMORY_SPY_CAUGHT,
	MEMORY_GIVE_HELP,
	MEMORY_REFUSED_HELP,
	MEMORY_ACCEPT_DEMAND,
	MEMORY_REJECTED_DEMAND,
	MEMORY_ACCEPTED_RELIGION,
	MEMORY_DENIED_RELIGION,
	MEMORY_ACCEPTED_CIVIC,
	MEMORY_DENIED_CIVIC,
	MEMORY_ACCEPTED_JOIN_WAR,
	MEMORY_DENIED_JOIN_WAR,
	MEMORY_ACCEPTED_STOP_TRADING,
	MEMORY_DENIED_STOP_TRADING,
	MEMORY_STOPPED_TRADING,
	MEMORY_STOPPED_TRADING_RECENT,
	MEMORY_HIRED_TRADE_EMBARGO,
	MEMORY_MADE_DEMAND,
	MEMORY_MADE_DEMAND_RECENT,
	MEMORY_CANCELLED_OPEN_BORDERS,
	MEMORY_TRADED_TECH_TO_US,
	MEMORY_RECEIVED_TECH_FROM_ANY,
	MEMORY_VOTED_AGAINST_US,
	MEMORY_VOTED_FOR_US,
	MEMORY_EVENT_GOOD_TO_US,
	MEMORY_EVENT_BAD_TO_US,
	MEMORY_LIBERATED_CITIES,

//FfH: Added by Kael 08/17/2008 (for Somnium)
	MEMORY_SOMNIUM_POSITIVE,
	MEMORY_SOMNIUM_NEGATIVE,
	MEMORY_SOMNIUM_DELAY,
//FfH: End Add
	MEMORY_MAGIC_CHARM,

	NUM_MEMORY_TYPES
};

enum AttitudeTypes				// Exposed to Python
{
	NO_ATTITUDE = -1,

	ATTITUDE_FURIOUS,
	ATTITUDE_ANNOYED,
	ATTITUDE_CAUTIOUS,
	ATTITUDE_PLEASED,
	ATTITUDE_FRIENDLY,

	NUM_ATTITUDE_TYPES
};

//! Enumeration for playing events with CvLeaderheadWidget
enum LeaderheadAction			// Exposed to Python
{
	NO_LEADERANIM = -1,		//!< If used with CvLeaderheadWidget::PerformAction, plays the idle animation

	LEADERANIM_GREETING,		//!< The leaderhead greets you
	LEADERANIM_FRIENDLY,		//!< The leaderhead is friendly to you
	LEADERANIM_PLEASED,			//!< The leaderhead is pleased
	LEADERANIM_CAUTIOUS,		//!< The leaderhead is cautious
	LEADERANIM_ANNOYED,			//!< The leaderhead is annoyed
	LEADERANIM_FURIOUS,			//!< The leaderhead is furious
	LEADERANIM_DISAGREE,		//!< The leaderhead makes a disagree gesture
	LEADERANIM_AGREE,				//!< The leaderhead makes an agree gesture

#ifdef _USRDLL
	NUM_LEADERANIM_TYPES			//!< The number of leaderhead event types
#endif
};

enum DiplomacyPowerTypes	// Exposed to Python
{
	NO_DIPLOMACYPOWER		= -1,
	DIPLOMACYPOWER_WEAKER,
	DIPLOMACYPOWER_EQUAL,
	DIPLOMACYPOWER_STRONGER,

	NUM_DIPLOMACYPOWER_TYPES
};

enum FeatTypes						// Exposed to Python
{
	FEAT_UNITCOMBAT_ARCHER,
	FEAT_UNITCOMBAT_MOUNTED,
	FEAT_UNITCOMBAT_MELEE,
	FEAT_UNITCOMBAT_SIEGE,
	FEAT_UNITCOMBAT_GUN,
	FEAT_UNITCOMBAT_ARMOR,
	FEAT_UNITCOMBAT_HELICOPTER,
	FEAT_UNITCOMBAT_NAVAL,
	FEAT_UNIT_PRIVATEER,
	FEAT_UNIT_SPY,
	FEAT_NATIONAL_WONDER,
	FEAT_TRADE_ROUTE,
	FEAT_COPPER_CONNECTED,
	FEAT_HORSE_CONNECTED,
	FEAT_IRON_CONNECTED,
	FEAT_LUXURY_CONNECTED,
	FEAT_FOOD_CONNECTED,

//FfH: Modified by Kael 10/06/2008
//	FEAT_POPULATION_HALF_MILLION,
//	FEAT_POPULATION_1_MILLION,
//	FEAT_POPULATION_2_MILLION,
//	FEAT_POPULATION_5_MILLION,
//	FEAT_POPULATION_10_MILLION,
//	FEAT_POPULATION_20_MILLION,
//	FEAT_POPULATION_50_MILLION,
//	FEAT_POPULATION_100_MILLION,
//	FEAT_POPULATION_200_MILLION,
//	FEAT_POPULATION_500_MILLION,
//	FEAT_POPULATION_1_BILLION,
//	FEAT_POPULATION_2_BILLION,
//	FEAT_CORPORATION_ENABLED,
//	FEAT_PAD,
	FEAT_CORPORATION_ENABLED,
	FEAT_PAD,
    FEAT_TRUST,
    FEAT_HEAL_UNIT_PER_TURN,
    FEAT_GLOBAL_SPELL,
//FfH: End Modify

/*************************************************************************************************/
/**	ADDON (New Functions Definition) Sephi                                   					**/
/*************************************************************************************************/
    FEAT_REPUBLIC_GAINED_AGGRESSIVE,
    FEAT_REPUBLIC_GAINED_DEFENDER,
    FEAT_REPUBLIC_GAINED_EXPANSIVE,
    FEAT_REPUBLIC_GAINED_FINANCIAL,
    FEAT_REPUBLIC_GAINED_PHILOSOPHICAL,
    FEAT_REPUBLIC_GAINED_INDUSTRIOUS,
    FEAT_REPUBLIC_GAINED_SPIRITUAL,
    FEAT_REPUBLIC_GAINED_ORGANIZED,
    FEAT_EXPEDITION_READY,
    FEAT_MANIFEST_HORNED_DREAD,
    FEAT_MANIFEST_FIRST_HORNED_DREAD,
    FEAT_ARTIFACT_1,
    FEAT_ARTIFACT_2,
    FEAT_ARTIFACT_3,
    FEAT_ARTIFACT_4,
/*************************************************************************************************/
/**	END	                                        												**/
/*************************************************************************************************/

	NUM_FEAT_TYPES
};

enum SaveGameTypes			// Exposed to Python
{
	SAVEGAME_NONE = -1,

	SAVEGAME_AUTO,
	SAVEGAME_RECOVERY,
	SAVEGAME_QUICK,
	SAVEGAME_NORMAL,
	SAVEGAME_GROUP,
	SAVEGAME_DROP_QUIT,
	SAVEGAME_DROP_CONTINUE,
	SAVEGAME_PBEM,
	SAVEGAME_REPLAY,

#ifdef _USRDLL
	NUM_SAVEGAME_TYPES
#endif
};

enum InitializationStates
{
	INIT_OK,
	INIT_FAILED,

#ifdef _USRDLL
	NUM_INIT_STATES
#endif
};

enum GameType				// Exposed to Python
{
	GAME_NONE = -1,

	GAME_SP_NEW,
	GAME_SP_SCENARIO,
	GAME_SP_LOAD,
	GAME_MP_NEW,
	GAME_MP_SCENARIO,
	GAME_MP_LOAD,
	GAME_HOTSEAT_NEW,
	GAME_HOTSEAT_SCENARIO,
	GAME_HOTSEAT_LOAD,
	GAME_PBEM_NEW,
	GAME_PBEM_SCENARIO,
	GAME_PBEM_LOAD,
	GAME_REPLAY,

#ifdef _USRDLL
	NUM_GAMETYPES
#endif
};

enum GameMode				// Exposed to Python
{
	NO_GAMEMODE = -1,

	GAMEMODE_NORMAL,
	GAMEMODE_PITBOSS,

#ifdef _USRDLL
	NUM_GAMEMODES
#endif
};

enum GamePwdTypes
{
	NO_PWDTYPE = -1,

	PWD_NEWGAME,
	PWD_SCENARIO,
	PWD_JOINGAME,

#ifdef _USRDLL
	NUM_PWDTYPES
#endif
};

enum SlotClaim
{
	SLOTCLAIM_UNASSIGNED,
	SLOTCLAIM_RESERVED,
	SLOTCLAIM_ASSIGNED,

#ifdef _USRDLL
	NUM_SLOTCLAIMS
#endif
};

enum SlotStatus
{
	SS_OPEN,
	SS_COMPUTER,
	SS_CLOSED,
	SS_TAKEN,

	SS_MAX_SLOT_STATUS,
};

enum InterfaceVisibility				// Exposed to Python
{
	INTERFACE_SHOW,
	INTERFACE_HIDE,
	INTERFACE_HIDE_ALL,
	INTERFACE_MINIMAP_ONLY,
	INTERFACE_ADVANCED_START,
};

enum GenericButtonSizes					// Exposed to Python
{
	BUTTON_SIZE_46,
	BUTTON_SIZE_32,
	BUTTON_SIZE_24,
	BUTTON_SIZE_16,

	BUTTON_SIZE_CUSTOM,
};

enum MenuScreenType
{
	MENU_SCREEN_STANDARD,
	MENU_SCREEN_BASELOBBY,
	MENU_SCREEN_BASE_SCENARIO,
	MENU_SCREEN_JOIN,
	MENU_SCREEN_BASE_LOAD,
	MENU_SCREEN_LAUNCHING,
	MENU_SCREEN_BASE_INIT,

	MENU_SCREEN_OPENING_MENU,
	MENU_SCREEN_MOD_MENU,

	MENU_SCREEN_GS_BUDDYLIST,
	MENU_SCREEN_GS_CHATROOM,
	MENU_SCREEN_GS_CHANNEL,
	MENU_SCREEN_GS_CHOOSE_SCENARIO,
	MENU_SCREEN_GS_SCENARIO,
	MENU_SCREEN_GS_NAVBAR,
	MENU_SCREEN_GS_LOAD_STAGING,
	MENU_SCREEN_GS_MOTD,
	MENU_SCREEN_GS_LOGIN,
	MENU_SCREEN_GS_LOAD,
	MENU_SCREEN_GS_JOIN,
	MENU_SCREEN_GS_INIT,
	MENU_SCREEN_GS_CUSTOM_GAMES,

	MENU_SCREEN_LANLOBBY,
	MENU_SCREEN_MP_CHOOSE_SCENARIO,
	MENU_SCREEN_MP_SCENARIO,
	MENU_SCREEN_MP_LOAD_STAGING,
	MENU_SCREEN_MP_LOAD,
	MENU_SCREEN_MP_INIT,
	MENU_SCREEN_MP_CHOICES,
	MENU_SCREEN_MP_JOIN,

	MENU_SCREEN_SP_WORLD_SIZE,
	MENU_SCREEN_SP_SCENARIO,
	MENU_SCREEN_SP_LOAD,
	MENU_SCREEN_SP_INIT,
	MENU_SCREEN_SP_CHOICES,
	MENU_SCREEN_SP_ADVANCED_LOAD,
	MENU_SCREEN_SP_SIMPLE_CIV_PICKER,
	MENU_SCREEN_SP_REPLAY,
	MENU_SCREEN_SP_MAP_SCRIPT,
	MENU_SCREEN_SP_DIFFICULTY,
};

enum WorldBuilderPopupTypes			// Exposed to Python
{
	WBPOPUP_NONE = -1,

	WBPOPUP_START = 200,
	WBPOPUP_CITY=WBPOPUP_START,		// first entry, starts at 0
	WBPOPUP_UNIT,
	WBPOPUP_PLAYER,
	WBPOPUP_PLOT,
	WBPOPUP_TERRAIN,
	WBPOPUP_FEATURE,
	WBPOPUP_IMPROVEMENT,
	WBPOPUP_GAME,

#ifdef _USRDLL
	NUM_WBPOPUP
#endif
};

enum EventType									// Exposed to Python
{
	// mouseEvent
	EVT_LBUTTONDOWN = 1,
	EVT_LBUTTONDBLCLICK,
	EVT_RBUTTONDOWN,
	EVT_BACK,
	EVT_FORWARD,

	// kbdEvent
	EVT_KEYDOWN,
	EVT_KEYUP,
};

// Different types of load
enum LoadType										// Exposed to Python
{
	LOAD_NORMAL,
	LOAD_INIT,
	LOAD_SETUP,
	LOAD_GAMETYPE,
	LOAD_REPLAY,
	LOAD_NORMAL_AND_GAMETYPE,
};

//	Available Fonts
enum FontTypes									// Exposed to Python
{
	TITLE_FONT,
	GAME_FONT,
	SMALL_FONT,
	MENU_FONT,
	MENU_HIGHLIGHT_FONT,
};

enum PanelStyles								// Exposed to Python
{
	// Built in styles
	PANEL_STYLE_STANDARD,
	PANEL_STYLE_SOLID,
	PANEL_STYLE_EMPTY,
	PANEL_STYLE_FLAT,
	PANEL_STYLE_IN,
	PANEL_STYLE_OUT,
	PANEL_STYLE_EXTERNAL,
	PANEL_STYLE_DEFAULT,

	// Civ specific styles
	PANEL_STYLE_CIVILPEDIA,
	PANEL_STYLE_STONE,
	PANEL_STYLE_UNITSTAT,
	PANEL_STYLE_BLUELARGE,
	PANEL_STYLE_BLUE50,

	PANEL_STYLE_TOPBAR,
	PANEL_STYLE_BOTTOMBAR,
	PANEL_STYLE_TECH,

	PANEL_STYLE_GAMEHUD_LEFT,
	PANEL_STYLE_GAMEHUD_RIGHT,
	PANEL_STYLE_GAMEHUD_CENTER,
	PANEL_STYLE_GAMEHUD_STATS,
	PANEL_STYLE_GAME_MAP,
	PANEL_STYLE_GAME_TOPBAR,
	PANEL_STYLE_HUD_HELP,

	PANEL_STYLE_CITY_LEFT,
	PANEL_STYLE_CITY_RIGHT,
	PANEL_STYLE_CITY_TOP,
	PANEL_STYLE_CITY_TANSHADE,
	PANEL_STYLE_CITY_INFO,
	PANEL_STYLE_CITY_TANTL,
	PANEL_STYLE_CITY_TANTR,
	PANEL_STYLE_CITY_COLUMNL,
	PANEL_STYLE_CITY_COLUMNC,
	PANEL_STYLE_CITY_COLUMNR,
	PANEL_STYLE_CITY_TITLE,

	PANEL_STYLE_DAWN,
	PANEL_STYLE_DAWNTOP,
	PANEL_STYLE_DAWNBOTTOM,

	PANEL_STYLE_MAIN,
	PANEL_STYLE_MAIN_BLACK25,
	PANEL_STYLE_MAIN_BLACK50,
	PANEL_STYLE_MAIN_WHITE,
	PANEL_STYLE_MAIN_WHITETAB,
	PANEL_STYLE_MAIN_TAN,
	PANEL_STYLE_MAIN_TAN15,
	PANEL_STYLE_MAIN_TANL,
	PANEL_STYLE_MAIN_TANR,
	PANEL_STYLE_MAIN_TANT,
	PANEL_STYLE_MAIN_TANB,
	PANEL_STYLE_MAIN_BOTTOMBAR,
	PANEL_STYLE_MAIN_SELECT,
};

enum ButtonStyles								// Exposed to Python
{
	BUTTON_STYLE_STANDARD,
	BUTTON_STYLE_ETCHED,
	BUTTON_STYLE_FLAT,
	BUTTON_STYLE_IMAGE,
	BUTTON_STYLE_LABEL,
	BUTTON_STYLE_LINK,
	BUTTON_STYLE_SQUARE,
	BUTTON_STYLE_TOOL,
	BUTTON_STYLE_DEFAULT,

	// Civ specific styles
	BUTTON_STYLE_CIRCLE,
	BUTTON_STYLE_CITY_B01,
	BUTTON_STYLE_CITY_B02TL,
	BUTTON_STYLE_CITY_B02TR,
	BUTTON_STYLE_CITY_B02BL,
	BUTTON_STYLE_CITY_B02BR,
	BUTTON_STYLE_CITY_B03TL,
	BUTTON_STYLE_CITY_B03TC,
	BUTTON_STYLE_CITY_B03TR,
	BUTTON_STYLE_CITY_B03BL,
	BUTTON_STYLE_CITY_B03BC,
	BUTTON_STYLE_CITY_B03BR,
	BUTTON_STYLE_CITY_FLAT,
	BUTTON_STYLE_CITY_PLUS,
	BUTTON_STYLE_CITY_MINUS,

	BUTTON_STYLE_ARROW_LEFT,
	BUTTON_STYLE_ARROW_RIGHT
};

enum TableStyles								// Exposed to Python
{
	TABLE_STYLE_STANDARD,
	TABLE_STYLE_EMPTY,
	TABLE_STYLE_ALTEMPTY,
	TABLE_STYLE_CITY,
	TABLE_STYLE_EMPTYSELECTINACTIVE,
	TABLE_STYLE_ALTDEFAULT,
	TABLE_STYLE_STAGINGROOM
};

enum EventContextTypes					// Exposed to Python
{
	NO_EVENTCONTEXT = -1,

	EVENTCONTEXT_SELF,
	EVENTCONTEXT_ALL,
};

enum CivLoginStates
{
	NO_CIV_LOGIN = -1,

	LOGIN_CIV_OK,
	LOGIN_CIV_BAD_PWD,
	LOGIN_CIV_TAKEN,
	LOGIN_CIV_HAS_ID,
};

enum VersionTypes
{
	NO_VERSIONTYPE = -1,

	VERSIONTYPE_EXE,
	VERSIONTYPE_DLL,
	VERSIONTYPE_SHADERS,
	VERSIONTYPE_PYTHON,
	VERSIONTYPE_XML,
};

enum VoteResultTypes
{
	VOTERESULT_MAJORITY,
	VOTERESULT_UNANIMOUS,
};

enum VoteStatusTypes
{
	NO_VOTESTATUS = -1,

	VOTESTATUS_TIED = -2,
	VOTESTATUS_UNDECIDED = -3,
	VOTESTATUS_INCOMPLETE = -4
};

// Tab Group in Options Menu
enum TabGroupTypes				// Exposed to Python
{
	NO_TABGROUP = -1,

	TABGROUP_GAME,
	TABGROUP_INPUT,
	TABGROUP_GRAPHICS,
	TABGROUP_AUDIO,
	TABGROUP_CLOCK,

#ifdef _USRDLL
	NUM_TABGROUPS
#endif
};

enum ReplayMessageTypes		// Exposed to Python
{
	NO_REPLAY_MESSAGE = -1,

	REPLAY_MESSAGE_MAJOR_EVENT,
	REPLAY_MESSAGE_CITY_FOUNDED,
	REPLAY_MESSAGE_PLOT_OWNER_CHANGE,

#ifdef _USRDLL
	NUM_REPLAY_MESSAGE_TYPES
#endif
};

/*------------------------------------------------------------------------------------
Enum:		EAudioTag
Purpose:	To enumerate all of the tag strings loaded from the csv or xml files
------------------------------------------------------------------------------------*/
enum AudioTag							// Exposed to Python
{
	AUDIOTAG_NONE = -1,
	AUDIOTAG_SOUNDID,
	AUDIOTAG_CONTEXTID,
	AUDIOTAG_SOUNDTYPE,
	AUDIOTAG_2DSCRIPT,
	AUDIOTAG_3DSCRIPT,
	AUDIOTAG_SOUNDSCAPE,
	AUDIOTAG_POSITION,
	AUDIOTAG_SCRIPTTYPE,
	AUDIOTAG_LOADTYPE,
	AUDIOTAG_COUNT,
};

enum CivilopediaPageTypes		// Exposed to Python
{
	NO_CIVILOPEDIA_PAGE = -1,

	CIVILOPEDIA_PAGE_TECH,
	CIVILOPEDIA_PAGE_UNIT,

//FfH: Added by Kael 08/10/2007
	CIVILOPEDIA_PAGE_HERO,
//FfH: End Add

	CIVILOPEDIA_PAGE_BUILDING,
	CIVILOPEDIA_PAGE_WONDER,
	CIVILOPEDIA_PAGE_TERRAIN,
	CIVILOPEDIA_PAGE_FEATURE,

//FfH: Added by Kael 08/10/2007
	CIVILOPEDIA_PAGE_UNIQUE_FEATURE,
//FfH: End Add

	CIVILOPEDIA_PAGE_BONUS,
	CIVILOPEDIA_PAGE_IMPROVEMENT,
	CIVILOPEDIA_PAGE_SPECIALIST,

//FfH: Added by Kael 08/27/2008
	CIVILOPEDIA_PAGE_EFFECT,
//>>>>BUGFfH: Added by Denev 2009/10/14
//	CIVILOPEDIA_PAGE_ITEM,
	CIVILOPEDIA_PAGE_EQUIPMENT,
//FfH: End Modify
	CIVILOPEDIA_PAGE_RACE,
//FfH: End Add

	CIVILOPEDIA_PAGE_PROMOTION,

//FfH: Added by Kael 07/23/2007
	CIVILOPEDIA_PAGE_SPELL,
//FfH: End Add
//>>>>BUGFfH: Added by Denev 2009/09/19
	CIVILOPEDIA_PAGE_ABILITY,
//<<<<BUGFfH: End Add
	CIVILOPEDIA_PAGE_UNIT_GROUP,
	CIVILOPEDIA_PAGE_CIV,
	CIVILOPEDIA_PAGE_LEADER,
//>>>>BUGFfH: Added by Denev 2009/09/19
	CIVILOPEDIA_PAGE_TRAIT,
//<<<<BUGFfH: End Add
	CIVILOPEDIA_PAGE_RELIGION,
//>>>>BUGFfH: Moved to below(*1) by Denev 2009/09/19
//	CIVILOPEDIA_PAGE_CORPORATION,
//<<<<BUGFfH: End Move
	CIVILOPEDIA_PAGE_CIVIC,
	CIVILOPEDIA_PAGE_PROJECT,
	CIVILOPEDIA_PAGE_CONCEPT,
//>>>>BUGFfH: Moved to below(*2) by Denev 2009/09/19
//	CIVILOPEDIA_PAGE_CONCEPT_NEW,
//<<<<BUGFfH: End Move
	CIVILOPEDIA_PAGE_HINTS,
//>>>>BUGFfH: Moved from above(*1)(*2) by Denev 2009/09/19
	CIVILOPEDIA_PAGE_CORPORATION,
	CIVILOPEDIA_PAGE_CONCEPT_NEW,
//<<<<BUGFfH: End Move

//>>>>BUGFfH: Added by Denev 2009/09/19
	CIVILOPEDIA_PAGE_HISTORY,
//<<<<BUGFfH: End Add
/*************************************************************************************************/
/**	ADDON (New Pedia Categories) Sephi                                      					**/
/*************************************************************************************************/
	CIVILOPEDIA_PAGE_CONCEPT_WILDMANA,
	CIVILOPEDIA_PAGE_WILDMANA_GUIDE,
/*************************************************************************************************/
/**	END	                                        												**/
/*************************************************************************************************/

	NUM_CIVILOPEDIA_PAGE_TYPES
};

enum ActionSubTypes					// Exposed to Python
{
	NO_ACTIONSUBTYPE = -1,

	ACTIONSUBTYPE_INTERFACEMODE,
	ACTIONSUBTYPE_COMMAND,
	ACTIONSUBTYPE_BUILD,
	ACTIONSUBTYPE_PROMOTION,

//FfH: Added by Kael 07/23/2007
	ACTIONSUBTYPE_SPELL,
//FfH: End Add
/*************************************************************************************************/
/**	ADDON (automatic Spellcasting) Sephi                                     					**/
/**																								**/
/**						                                            							**/
/*************************************************************************************************/
    ACTIONSUBTYPE_AUTOMATE_SPELL,

/*************************************************************************************************/
/**	END	                                        												**/
/*************************************************************************************************/
	ACTIONSUBTYPE_UNIT,
	ACTIONSUBTYPE_RELIGION,
	ACTIONSUBTYPE_CORPORATION,
	ACTIONSUBTYPE_SPECIALIST,
	ACTIONSUBTYPE_BUILDING,
	ACTIONSUBTYPE_CONTROL,
	ACTIONSUBTYPE_AUTOMATE,
	ACTIONSUBTYPE_MISSION,

#ifdef _USRDLL
	NUM_ACTIONSUBTYPES
#endif
};

enum GameMessageTypes				// Exposed to Python
{
	GAMEMESSAGE_NETWORK_READY,
	GAMEMESSAGE_SAVE_GAME_FLAG,
	GAMEMESSAGE_SAVE_FLAG_ACK,
	GAMEMESSAGE_VERIFY_VERSION,
	GAMEMESSAGE_VERSION_NACK,
	GAMEMESSAGE_VERSION_WARNING,
	GAMEMESSAGE_GAME_TYPE,
	GAMEMESSAGE_ID_ASSIGNMENT,
	GAMEMESSAGE_FILE_INFO,
	GAMEMESSAGE_PICK_YOUR_CIV,
	GAMEMESSAGE_CIV_CHOICE,
	GAMEMESSAGE_CONFIRM_CIV_CLAIM,
	GAMEMESSAGE_CLAIM_INFO,
	GAMEMESSAGE_CIV_CHOICE_ACK,
	GAMEMESSAGE_CIV_CHOICE_NACK,
	GAMEMESSAGE_CIV_CHOSEN,
	GAMEMESSAGE_INTERIM_NOTICE,
	GAMEMESSAGE_INIT_INFO,
	GAMEMESSAGE_MAPSCRIPT_CHECK,
	GAMEMESSAGE_MAPSCRIPT_ACK,
	GAMEMESSAGE_LOAD_GAME,
	GAMEMESSAGE_PLAYER_ID,
	GAMEMESSAGE_SLOT_REASSIGNMENT,
	GAMEMESSAGE_PLAYER_INFO,
	GAMEMESSAGE_GAME_INFO,
	GAMEMESSAGE_REASSIGN_PLAYER,
	GAMEMESSAGE_PITBOSS_INFO,
	GAMEMESSAGE_LAUNCHING_INFO,
	GAMEMESSAGE_INIT_GAME,
	GAMEMESSAGE_INIT_PLAYERS,
	GAMEMESSAGE_AUTH_REQUEST,
	GAMEMESSAGE_AUTH_RESPONSE,
	GAMEMESSAGE_SYNCH_START,
	GAMEMESSAGE_PLAYER_OPTION,
	GAMEMESSAGE_EXTENDED_GAME,
	GAMEMESSAGE_AUTO_MOVES,
	GAMEMESSAGE_TURN_COMPLETE,
	GAMEMESSAGE_JOIN_GROUP,
	GAMEMESSAGE_PUSH_MISSION,
	GAMEMESSAGE_AUTO_MISSION,
	GAMEMESSAGE_DO_COMMAND,
	GAMEMESSAGE_PUSH_ORDER,
	GAMEMESSAGE_POP_ORDER,
	GAMEMESSAGE_DO_TASK,
	GAMEMESSAGE_RESEARCH,
	GAMEMESSAGE_PERCENT_CHANGE,
	GAMEMESSAGE_ESPIONAGE_CHANGE,
	GAMEMESSAGE_CONVERT,
	GAMEMESSAGE_CHAT,
	GAMEMESSAGE_PING,
	GAMEMESSAGE_SIGN,
	GAMEMESSAGE_LINE_ENTITY,
	GAMEMESSAGE_SIGN_DELETE,
	GAMEMESSAGE_LINE_ENTITY_DELETE,
	GAMEMESSAGE_LINE_GROUP_DELETE,
	GAMEMESSAGE_PAUSE,
	GAMEMESSAGE_MP_KICK,
	GAMEMESSAGE_MP_RETIRE,
	GAMEMESSAGE_CLOSE_CONNECTION,
	GAMEMESSAGE_NEVER_JOINED,
	GAMEMESSAGE_MP_DROP_INIT,
	GAMEMESSAGE_MP_DROP_VOTE,
	GAMEMESSAGE_MP_DROP_UPDATE,
	GAMEMESSAGE_MP_DROP_RESULT,
	GAMEMESSAGE_MP_DROP_SAVE,
	GAMEMESSAGE_TOGGLE_TRADE,
	GAMEMESSAGE_IMPLEMENT_OFFER,
	GAMEMESSAGE_CHANGE_WAR,
	GAMEMESSAGE_CHANGE_VASSAL,
	GAMEMESSAGE_CHOOSE_ELECTION,
	GAMEMESSAGE_DIPLO_VOTE,
	GAMEMESSAGE_APPLY_EVENT,
	GAMEMESSAGE_CONTACT_CIV,
	GAMEMESSAGE_DIPLO_CHAT,
	GAMEMESSAGE_SEND_OFFER,
	GAMEMESSAGE_DIPLO_EVENT,
	GAMEMESSAGE_RENEGOTIATE,
	GAMEMESSAGE_RENEGOTIATE_ITEM,
	GAMEMESSAGE_EXIT_TRADE,
	GAMEMESSAGE_KILL_DEAL,
	GAMEMESSAGE_SAVE_GAME,
	GAMEMESSAGE_UPDATE_CIVICS,
	GAMEMESSAGE_CLEAR_TABLE,
	GAMEMESSAGE_POPUP_PROCESSED,
	GAMEMESSAGE_DIPLOMACY_PROCESSED,
	GAMEMESSAGE_HOT_JOIN_NOTICE,
	GAMEMESSAGE_HOT_DROP_NOTICE,
	GAMEMESSAGE_DIPLOMACY,
	GAMEMESSAGE_POPUP,
	GAMEMESSAGE_EVENT_TRIGGERED,
	GAMEMESSAGE_EMPIRE_SPLIT,
	GAMEMESSAGE_LAUNCH_SPACESHIP,
	GAMEMESSAGE_ADVANCED_START_ACTION,
	GAMEMESSAGE_FOUND_RELIGION,
	GAMEMESSAGE_MOD_NET_MESSAGE
};

enum PopupControlLayout		// Exposed to Python
{
	POPUP_LAYOUT_LEFT,
	POPUP_LAYOUT_CENTER,
	POPUP_LAYOUT_RIGHT,
	POPUP_LAYOUT_STRETCH,

	POPUP_LAYOUT_NUMLAYOUTS
};

enum JustificationTypes		// Exposed to Python
{
	DLL_FONT_LEFT_JUSTIFY = 1<<0,
	DLL_FONT_RIGHT_JUSTIFY = 1<<1,
	DLL_FONT_CENTER_JUSTIFY = 1<<2,
	DLL_FONT_CENTER_VERTICALLY = 1<<3,
	DLL_FONT_ADDITIVE = 1<<4,
};

enum ToolTipAlignTypes		// Exposed to Python
{
	TOOLTIP_TOP_LEFT,
	TOOLTIP_TOP_INLEFT,
	TOOLTIP_TOP_CENTER,
	TOOLTIP_TOP_INRIGHT,
	TOOLTIP_TOP_RIGHT,

	TOOLTIP_INTOP_RIGHT,
	TOOLTIP_CENTER_RIGHT,
	TOOLTIP_INBOTTOM_RIGHT,

	TOOLTIP_BOTTOM_RIGHT,
	TOOLTIP_BOTTOM_INRIGHT,
	TOOLTIP_BOTTOM_CENTER,
	TOOLTIP_BOTTOM_INLEFT,
	TOOLTIP_BOTTOM_LEFT,

	TOOLTIP_INBOTTOM_LEFT,
	TOOLTIP_CENTER_LEFT,
	TOOLTIP_INTOP_LEFT,
};

enum ActivationTypes			// Exposed to Python
{
	ACTIVATE_NORMAL,
	ACTIVATE_CHILDFOCUS,
	ACTIVATE_MIMICPARENT,
	ACTIVATE_MIMICPARENTFOCUS,
};

enum HitTestTypes					// Exposed to Python
{
	HITTEST_DEFAULT,
	HITTEST_ON,
	HITTEST_SOLID,
	HITTEST_CHILDREN,
	HITTEST_NOHIT,
};

enum GraphicLevelTypes		// Exposed to Python
{
	GRAPHICLEVEL_HIGH,
	GRAPHICLEVEL_MEDIUM,
	GRAPHICLEVEL_LOW,
	GRAPHICLEVEL_CURRENT,

#ifdef _USRDLL
	NUM_GRAPHICLEVELS
#endif
};

enum EventTypes		// Exposed to Python
{
	NO_EVENT = -1,
};

enum EventTriggerTypes		// Exposed to Python
{
	NO_EVENTTRIGGER = -1,
};

enum EspionageMissionTypes				// Exposed to Python
{
	NO_ESPIONAGEMISSION = -1,
};

enum AdvancedStartActionTypes				// Exposed to Python
{
	NO_ADVANCEDSTARTACTION = -1,

	ADVANCEDSTARTACTION_EXIT,
	ADVANCEDSTARTACTION_UNIT,
	ADVANCEDSTARTACTION_CITY,
	ADVANCEDSTARTACTION_POP,
	ADVANCEDSTARTACTION_CULTURE,
	ADVANCEDSTARTACTION_BUILDING,
	ADVANCEDSTARTACTION_IMPROVEMENT,
	ADVANCEDSTARTACTION_ROUTE,
	ADVANCEDSTARTACTION_TECH,
	ADVANCEDSTARTACTION_VISIBILITY,
	ADVANCEDSTARTACTION_AUTOMATE
};

enum GlobeLayerTypes
{
	GLOBE_LAYER_STRATEGY = -1,
	GLOBE_LAYER_TRADE,
	GLOBE_LAYER_UNIT,
	GLOBE_LAYER_RESOURCE,
	GLOBE_LAYER_RELIGION,
	GLOBE_LAYER_CULTURE,

#ifdef _USRDLL
	NUM_GLOBE_LAYER_TYPES
#endif
};

enum GlobeLayerUnitOptionTypes
{
	SHOW_ALL_MILITARY,
	SHOW_TEAM_MILITARY,
	SHOW_ENEMIES_IN_TERRITORY,
	SHOW_ENEMIES,
	SHOW_PLAYER_DOMESTICS,

#ifdef _USRDLL
	NUM_UNIT_OPTION_TYPES
#endif
};

enum GlobeLayerResourceOptionTypes
{
	SHOW_ALL_RESOURCES,
	SHOW_STRATEGIC_RESOURCES,
	SHOW_HAPPY_RESOURCES,
	SHOW_HEALTH_RESOURCES,

#ifdef _USRDLL
	NUM_RESOURCE_OPTION_TYPES
#endif
};

enum PlotIndicatorVisibilityFlags
{
	PLOT_INDICATOR_VISIBLE_ALWAYS,
	PLOT_INDICATOR_VISIBLE_ONSCREEN_ONLY,
	PLOT_INDICATOR_VISIBLE_OFFSCREEN_ONLY,
	PLOT_INDICATOR_VISIBLE_ONGLOBE_ONLY,
	PLOT_INDICATOR_VISIBLE_NEVER
};

enum UnitSubEntityTypes
{
	UNIT_SUB_ENTITY_NORMAL,
	UNIT_SUB_ENTITY_LEADER,
	UNIT_SUB_ENTITY_SIEGE_TOWER,
	UNIT_SUB_ENTITY_COUNT
};

enum CivilopediaWidgetShowTypes
{
	CIVILOPEDIA_WIDGET_SHOW_NONE,
	CIVILOPEDIA_WIDGET_SHOW_LAND,
	CIVILOPEDIA_WIDGET_SHOW_WATER,
};

enum DungeonPopulationLevels
{
	DUNGEON_POPULATION_LOW,
	DUNGEON_POPULATION_AVERAGE,
	DUNGEON_POPULATION_HIGH,
	DUNGEON_POPULATION_VERY_HIGH,
};

enum DungeonPowerLevels
{
	DUNGEON_POWER_WEAK,
	DUNGEON_POWER_AVERAGE,
	DUNGEON_POWER_STRONG,
	DUNGEON_POWER_DEADLY,
};

#endif	// CVENUMS_h
