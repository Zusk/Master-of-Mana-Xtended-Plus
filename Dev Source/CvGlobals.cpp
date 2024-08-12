//
// globals.cpp
//
#include "CvGameCoreDLL.h"
#include "CvGlobals.h"
#include "CvRandom.h"
#include "CvGameAI.h"
#include "CvDLLInterfaceIFaceBase.h"
#include "CvMap.h"
#include "CvPlayerAI.h"
#include "CvTeamAI.h"
#include "CvInfos.h"
#include "CvDLLUtilityIFaceBase.h"
#include "CvArtFileMgr.h"
#include "CvDLLXMLIFaceBase.h"
#include "CvPlayerAI.h"
#include "CvInfoWater.h"
#include "CvGameTextMgr.h"
#include "FProfiler.h"
#include "FVariableSystem.h"
#include "CvInitCore.h"
/*************************************************************************************************/
/**	TrueModular								05/26/09	Written: Mr. Genie	Imported: Xienwolf	**/
/**					Imports some new tools to utilize for various purposes						**/
/**	Properly links Modular modifications to previous elements, and allows partial overwriting	**/
/*************************************************************************************************/
#include "CvXMLLoadUtility.h"
/*************************************************************************************************/
/**	TrueModular								END													**/
/*************************************************************************************************/
#include "TimeMeasure.h"
/*************************************************************************************************/
/**	ADDON (WINAMP) merged Sephi																	**/
/*************************************************************************************************/
#include "CvWinamp.h"
/*************************************************************************************************/
/**	ADDON (WINAMP)						END														**/
/*************************************************************************************************/

#define COPY(dst, src, typeName) \
	{ \
		int iNum = sizeof(src)/sizeof(typeName); \
		dst = new typeName[iNum]; \
		for (int i =0;i<iNum;i++) \
			dst[i] = src[i]; \
	}

template <class T>
void deleteInfoArray(std::vector<T*>& array)
{
	for (std::vector<T*>::iterator it = array.begin(); it != array.end(); ++it)
	{
		SAFE_DELETE(*it);
	}

	array.clear();
}

template <class T>
bool readInfoArray(FDataStreamBase* pStream, std::vector<T*>& array, const char* szClassName)
{
	GC.addToInfosVectors(&array);

	int iSize;
	pStream->Read(&iSize);
	FAssertMsg(iSize==sizeof(T), CvString::format("class size doesn't match cache size - check info read/write functions:%s", szClassName).c_str());
	if (iSize!=sizeof(T))
		return false;
	pStream->Read(&iSize);

	deleteInfoArray(array);

	for (int i = 0; i < iSize; ++i)
	{
		array.push_back(new T);
	}

	int iIndex = 0;
	for (std::vector<T*>::iterator it = array.begin(); it != array.end(); ++it)
	{
		(*it)->read(pStream);
		GC.setInfoTypeFromString((*it)->getType(), iIndex);
		++iIndex;
	}

	return true;
}

template <class T>
bool writeInfoArray(FDataStreamBase* pStream,  std::vector<T*>& array)
{
	int iSize = sizeof(T);
	pStream->Write(iSize);
	pStream->Write(array.size());
	for (std::vector<T*>::iterator it = array.begin(); it != array.end(); ++it)
	{
		(*it)->write(pStream);
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////

CvGlobals gGlobals;

//
// CONSTRUCTOR
//
CvGlobals::CvGlobals() :
m_bGraphicsInitialized(false),
m_bLogging(false),
m_bRandLogging(false),
m_bOverwriteLogs(false),
m_bSynchLogging(false),
m_bDLLProfiler(false),
m_pkMainMenu(NULL),
m_iNewPlayers(0),
m_bZoomOut(false),
m_bZoomIn(false),
m_bLoadGameFromFile(false),
m_pFMPMgr(NULL),
m_asyncRand(NULL),
m_interface(NULL),
m_game(NULL),
m_messageQueue(NULL),
m_hotJoinMsgQueue(NULL),
m_messageControl(NULL),
m_messageCodes(NULL),
m_dropMgr(NULL),
m_portal(NULL),
m_setupData(NULL),
m_initCore(NULL),
m_statsReporter(NULL),
m_map(NULL),
m_diplomacyScreen(NULL),
m_mpDiplomacyScreen(NULL),
m_pathFinder(NULL),
m_interfacePathFinder(NULL),
m_stepFinder(NULL),
m_routeFinder(NULL),
m_borderFinder(NULL),
m_areaFinder(NULL),
m_plotGroupFinder(NULL),
m_stepUnitFinder(NULL),
m_pDLL(NULL),
m_aiPlotDirectionX(NULL),
m_aiPlotDirectionY(NULL),
m_aiPlotCardinalDirectionX(NULL),
m_aiPlotCardinalDirectionY(NULL),
m_aiCityPlotX(NULL),
m_aiCityPlotY(NULL),
m_aiCityPlotPriority(NULL),
m_aeTurnLeftDirection(NULL),
m_aeTurnRightDirection(NULL),
//m_aGameOptionsInfo(NULL),
//m_aPlayerOptionsInfo(NULL),
m_Profiler(NULL),
m_VarSystem(NULL),
m_iMOVE_DENOMINATOR(0),
m_iNUM_UNIT_PREREQ_OR_BONUSES(0),
m_iNUM_BUILDING_PREREQ_OR_BONUSES(0),
m_iFOOD_CONSUMPTION_PER_POPULATION(0),
m_iMAX_HIT_POINTS(0),
m_iPATH_DAMAGE_WEIGHT(0),
m_iHILLS_EXTRA_DEFENSE(0),
m_iRIVER_ATTACK_MODIFIER(0),
m_iAMPHIB_ATTACK_MODIFIER(0),
m_iHILLS_EXTRA_MOVEMENT(0),
m_iMAX_PLOT_LIST_ROWS(0),
m_iUNIT_MULTISELECT_MAX(0),
m_iPERCENT_ANGER_DIVISOR(0),
m_iEVENT_MESSAGE_TIME(0),
m_iROUTE_FEATURE_GROWTH_MODIFIER(0),
m_iFEATURE_GROWTH_MODIFIER(0),
m_iMIN_CITY_RANGE(0),
m_iCITY_MAX_NUM_BUILDINGS(0),
m_iNUM_UNIT_AND_TECH_PREREQS(0),
m_iNUM_AND_TECH_PREREQS(0),
m_iNUM_OR_TECH_PREREQS(0),
m_iLAKE_MAX_AREA_SIZE(0),
m_iNUM_ROUTE_PREREQ_OR_BONUSES(0),
m_iNUM_BUILDING_AND_TECH_PREREQS(0),
m_iMIN_WATER_SIZE_FOR_OCEAN(0),
m_iFORTIFY_MODIFIER_PER_TURN(0),
m_iMAX_CITY_DEFENSE_DAMAGE(0),
m_iNUM_CORPORATION_PREREQ_BONUSES(0),
m_iPEAK_SEE_THROUGH_CHANGE(0),
m_iHILLS_SEE_THROUGH_CHANGE(0),
m_iSEAWATER_SEE_FROM_CHANGE(0),
m_iPEAK_SEE_FROM_CHANGE(0),
m_iHILLS_SEE_FROM_CHANGE(0),
m_iUSE_SPIES_NO_ENTER_BORDERS(0),
m_fCAMERA_MIN_YAW(0),
m_fCAMERA_MAX_YAW(0),
m_fCAMERA_FAR_CLIP_Z_HEIGHT(0),
m_fCAMERA_MAX_TRAVEL_DISTANCE(0),
m_fCAMERA_START_DISTANCE(0),
m_fAIR_BOMB_HEIGHT(0),
m_fPLOT_SIZE(0),
m_fCAMERA_SPECIAL_PITCH(0),
m_fCAMERA_MAX_TURN_OFFSET(0),
m_fCAMERA_MIN_DISTANCE(0),
m_fCAMERA_UPPER_PITCH(0),
m_fCAMERA_LOWER_PITCH(0),
m_fFIELD_OF_VIEW(0),
m_fSHADOW_SCALE(0),
m_fUNIT_MULTISELECT_DISTANCE(0),
m_iUSE_CANNOT_FOUND_CITY_CALLBACK(0),
m_iUSE_CAN_FOUND_CITIES_ON_WATER_CALLBACK(0),
m_iUSE_IS_PLAYER_RESEARCH_CALLBACK(0),
m_iUSE_CAN_RESEARCH_CALLBACK(0),
m_iUSE_CANNOT_DO_CIVIC_CALLBACK(0),
m_iUSE_CAN_DO_CIVIC_CALLBACK(0),
m_iUSE_CANNOT_CONSTRUCT_CALLBACK(0),
m_iUSE_CAN_CONSTRUCT_CALLBACK(0),
m_iUSE_CAN_DECLARE_WAR_CALLBACK(0),
m_iUSE_CANNOT_RESEARCH_CALLBACK(0),
m_iUSE_GET_UNIT_COST_MOD_CALLBACK(0),
m_iUSE_GET_CITY_FOUND_VALUE_CALLBACK(0),
m_iUSE_CANNOT_HANDLE_ACTION_CALLBACK(0),
m_iUSE_CAN_BUILD_CALLBACK(0),
m_iUSE_CANNOT_TRAIN_CALLBACK(0),
m_iUSE_CAN_TRAIN_CALLBACK(0),
m_iUSE_UNIT_CANNOT_MOVE_INTO_CALLBACK(0),
m_iUSE_USE_CANNOT_SPREAD_RELIGION_CALLBACK(0),
m_iUSE_FINISH_TEXT_CALLBACK(0),
m_iUSE_ON_UNIT_SET_XY_CALLBACK(0),
m_iUSE_ON_UNIT_SELECTED_CALLBACK(0),
m_iUSE_ON_UPDATE_CALLBACK(0),
m_iUSE_ON_UNIT_CREATED_CALLBACK(0),
m_iUSE_ON_UNIT_LOST_CALLBACK(0),

//FfH: Added by Kael 08/02/2008
m_iUSE_ON_UNIT_MOVE_CALLBACK(0),
m_iUSE_PLOT_REVEALED_CALLBACK(0),
m_iUSE_COMBAT_RESULT_CALLBACK(0),
//FfH: End Add
m_iFREE_FAVORED_IMPROVEMENTS_PER_CITY(0),
m_iIMPROVEMENT_COTTAGE(-1),
m_iIMPROVEMENT_FARM(-1),
m_iIMPROVEMENT_GROUNDWATER_WELL(-1),
m_iIMPROVEMENT_PASTURE(-1),
m_iIMPROVEMENT_QUARRY(-1),
m_iIMPROVEMENT_WATERMILL(-1),
m_iIMPROVEMENT_WINDMILL(-1),
m_iIMPROVEMENT_WORKSHOP(-1),
m_iFEATURE_HAUNTED_LANDS(-1),
m_iBUILDINGCLASS_SAGE_DISTRICT(-1),
m_iBUILDINGCLASS_NOBLE_DISTRICT(-1),
m_iBUILDINGCLASS_MERCHANT_DISTRICT(-1),
m_iBUILDINGCLASS_WARRIOR_DISTRICT(-1),
m_iBUILDINGCLASS_TEMPLE_DISTRICT(-1),
m_iBUILDINGCLASS_BARD_DISTRICT(-1),
m_iBUILDINGCLASS_LUXURY_DISTRICT(-1),
m_iBUILDINGCLASS_FOREIGN_TRADE_DISTRICT(-1),
m_iBUILDINGCLASS_RESOURCE_DISTRICT(-1),
m_iBUILDINGCLASS_HERBALIST(-1),
m_iBUILDINGCLASS_SCHOLA_ARCANA(-1),
m_iBUILDINGCLASS_SALON(-1),
m_iBUILDINGCLASS_PALACE(-1),
m_iUNITCLASS_SETTLER(-1),
m_iUNITCLASS_WORKBOAT(-1),
m_iUNITCLASS_WORKER(-1),
m_iEQUIPMENT_PIECES_OF_BARNAXUS(-1),
m_iUNITCOMBAT_ADEPT(-1),
m_iSPECIALIST_MERCHANT(-1),
m_iSPECIALIST_SCIENTIST(-1),
m_iSPECIALIST_ARTIST(-1),
m_iSPECIALIST_PRIEST(-1),
m_iSPECIALIST_ENGINEER(-1),
m_iIMPROVEMENT_MANA_SHRINE(-1),
m_iIMPROVEMENT_TRADE1(-1),
m_iIMPROVEMENT_CAMP(-1),
m_iDEFAULT_SPECIALIST(-1),
m_TimeMeasure(NULL),
/*************************************************************************************************/
/**	ADDON (WINAMP) merged Sephi																	**/
/*************************************************************************************************/
m_Winamp(NULL),
/*************************************************************************************************/
/**	ADDON (WINAMP)						END														**/
/*************************************************************************************************/
m_iGraphicalDetailPageInRange(256),
m_paHints(NULL),
m_paMainMenus(NULL)
{
}

CvGlobals::~CvGlobals()
{
}

/************************************************************************************************/
/* MINIDUMP_MOD                           04/10/11                                terkhen       */
/*                                                                                              */
/* See http://www.debuginfo.com/articles/effminidumps.html                                      */
/************************************************************************************************/

#include <dbghelp.h>
#pragma comment (lib, "dbghelp.lib")

void CreateMiniDump(EXCEPTION_POINTERS *pep)
{
	/* Open a file to store the minidump. */
	HANDLE hFile = CreateFile(_T("MiniDump.dmp"),
	                          GENERIC_READ | GENERIC_WRITE,
	                          0,
	                          NULL,
	                          CREATE_ALWAYS,
	                          FILE_ATTRIBUTE_NORMAL,
	                          NULL);

	if((hFile == NULL) || (hFile == INVALID_HANDLE_VALUE)) {
		_tprintf(_T("CreateFile failed. Error: %u \n"), GetLastError());
		return;
	}
	/* Create the minidump. */
	MINIDUMP_EXCEPTION_INFORMATION mdei;

	mdei.ThreadId           = GetCurrentThreadId();
	mdei.ExceptionPointers  = pep;
	mdei.ClientPointers     = FALSE;

	MINIDUMP_TYPE mdt       = MiniDumpNormal;

	BOOL result = MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(),
	                                hFile,
	                                mdt,
	                                (pep != NULL) ? &mdei : NULL,
	                                NULL,
	                                NULL);

	/* Close the file. */
	CloseHandle(hFile);
}

LONG WINAPI CustomFilter(EXCEPTION_POINTERS *ExceptionInfo)
{
	CreateMiniDump(ExceptionInfo);
	return EXCEPTION_EXECUTE_HANDLER;
}

/************************************************************************************************/
/* MINIDUMP_MOD                                END                                              */
/************************************************************************************************/

//
// allocate
//
void CvGlobals::init()
{
/************************************************************************************************/
/* MINIDUMP_MOD                           04/10/11                                terkhen       */
/************************************************************************************************/

	/* Enable our custom exception that will write the minidump for us. */
	//SetUnhandledExceptionFilter(CustomFilter);

/************************************************************************************************/
/* MINIDUMP_MOD                                END                                              */
/************************************************************************************************/
	//
	// These vars are used to initialize the globals.
	//

	int aiPlotDirectionX[NUM_DIRECTION_TYPES] =
	{
		0,	// DIRECTION_NORTH
		1,	// DIRECTION_NORTHEAST
		1,	// DIRECTION_EAST
		1,	// DIRECTION_SOUTHEAST
		0,	// DIRECTION_SOUTH
		-1,	// DIRECTION_SOUTHWEST
		-1,	// DIRECTION_WEST
		-1,	// DIRECTION_NORTHWEST
	};

	int aiPlotDirectionY[NUM_DIRECTION_TYPES] =
	{
		1,	// DIRECTION_NORTH
		1,	// DIRECTION_NORTHEAST
		0,	// DIRECTION_EAST
		-1,	// DIRECTION_SOUTHEAST
		-1,	// DIRECTION_SOUTH
		-1,	// DIRECTION_SOUTHWEST
		0,	// DIRECTION_WEST
		1,	// DIRECTION_NORTHWEST
	};

	int aiPlotCardinalDirectionX[NUM_CARDINALDIRECTION_TYPES] =
	{
		0,	// CARDINALDIRECTION_NORTH
		1,	// CARDINALDIRECTION_EAST
		0,	// CARDINALDIRECTION_SOUTH
		-1,	// CARDINALDIRECTION_WEST
	};

	int aiPlotCardinalDirectionY[NUM_CARDINALDIRECTION_TYPES] =
	{
		1,	// CARDINALDIRECTION_NORTH
		0,	// CARDINALDIRECTION_EAST
		-1,	// CARDINALDIRECTION_SOUTH
		0,	// CARDINALDIRECTION_WEST
	};

	int aiCityPlotX[NUM_CITY_PLOTS] =
	{
		0,
		0, 1, 1, 1, 0,-1,-1,-1,
		0, 1, 2, 2, 2, 1, 0,-1,-2,-2,-2,-1,

//FfH: Added by Kael 11/18/2007
		0, 1, 2, 3, 3, 3, 2, 1, 0, -1, -2, -3, -3, -3, -2, -1,
//FfH: End Add

	};

	int aiCityPlotY[NUM_CITY_PLOTS] =
	{
		0,
		1, 1, 0,-1,-1,-1, 0, 1,
		2, 2, 1, 0,-1,-2,-2,-2,-1, 0, 1, 2,

//FfH: Added by Kael 11/18/2007
		3, 3, 2, 1, 0, -1, -2, -3, -3, -3, -2, -1, 0, 1, 2, 3,
//FfH: End Add

	};

	int aiCityPlotPriority[NUM_CITY_PLOTS] =
	{
		0,
		1, 2, 1, 2, 1, 2, 1, 2,
		3, 4, 4, 3, 4, 4, 3, 4, 4, 3, 4, 4,

//FfH: Added by Kael 11/18/2007
		5, 6, 7, 6, 5, 6, 7, 6, 5, 6, 7, 6, 5, 6, 7, 6,
//FfH: End Add

	};

	int aaiXYCityPlot[CITY_PLOTS_DIAMETER][CITY_PLOTS_DIAMETER] =
	{

//FfH: Modified by Kael 11/18/2007
//		{-1, 17, 18, 19, -1,},
//		{16, 6, 7, 8, 20,},
//		{15, 5, 0, 1, 9,},
//		{14, 4, 3, 2, 10,},
//		{-1, 13, 12, 11, -1,}
	    {-1, -1, 32, 33, 34, -1, -1},
	    {-1, 31, 17, 18, 19, 35, -1},
	    {30, 16, 6, 7, 8, 20, 36},
	    {29, 15, 5, 0, 1, 9, 21},
	    {28, 14, 4, 3, 2, 10, 22},
	    {-1, 27, 13, 12, 11, 23, -1},
	    {-1, -1, 26, 25, 24, -1, -1}
//FfH: End Modify

	};

	DirectionTypes aeTurnRightDirection[NUM_DIRECTION_TYPES] =
	{
		DIRECTION_NORTHEAST,	// DIRECTION_NORTH
		DIRECTION_EAST,				// DIRECTION_NORTHEAST
		DIRECTION_SOUTHEAST,	// DIRECTION_EAST
		DIRECTION_SOUTH,			// DIRECTION_SOUTHEAST
		DIRECTION_SOUTHWEST,	// DIRECTION_SOUTH
		DIRECTION_WEST,				// DIRECTION_SOUTHWEST
		DIRECTION_NORTHWEST,	// DIRECTION_WEST
		DIRECTION_NORTH,			// DIRECTION_NORTHWEST
	};

	DirectionTypes aeTurnLeftDirection[NUM_DIRECTION_TYPES] =
	{
		DIRECTION_NORTHWEST,	// DIRECTION_NORTH
		DIRECTION_NORTH,			// DIRECTION_NORTHEAST
		DIRECTION_NORTHEAST,	// DIRECTION_EAST
		DIRECTION_EAST,				// DIRECTION_SOUTHEAST
		DIRECTION_SOUTHEAST,	// DIRECTION_SOUTH
		DIRECTION_SOUTH,			// DIRECTION_SOUTHWEST
		DIRECTION_SOUTHWEST,	// DIRECTION_WEST
		DIRECTION_WEST,				// DIRECTION_NORTHWEST
	};

	DirectionTypes aaeXYDirection[DIRECTION_DIAMETER][DIRECTION_DIAMETER] =
	{
		DIRECTION_SOUTHWEST, DIRECTION_WEST,	DIRECTION_NORTHWEST,
		DIRECTION_SOUTH,     NO_DIRECTION,    DIRECTION_NORTH,
		DIRECTION_SOUTHEAST, DIRECTION_EAST,	DIRECTION_NORTHEAST,
	};

	FAssertMsg(gDLL != NULL, "Civ app needs to set gDLL");

	m_VarSystem = new FVariableSystem;
	m_asyncRand = new CvRandom;
	m_initCore = new CvInitCore;
	m_loadedInitCore = new CvInitCore;
	m_iniInitCore = new CvInitCore;

	gDLL->initGlobals();	// some globals need to be allocated outside the dll

	m_game = new CvGameAI;
	m_map = new CvMap;

	m_TimeMeasure = new TimeMeasure();
/*************************************************************************************************/
/**	ADDON (WINAMP) merged Sephi																	**/
/*************************************************************************************************/
	m_Winamp = new CvWinamp;
/*************************************************************************************************/
/**	ADDON (WINAMP)						END														**/
/*************************************************************************************************/

	CvPlayerAI::initStatics();
	CvTeamAI::initStatics();

	m_pt3Origin = NiPoint3(0.0f, 0.0f, 0.0f);

	COPY(m_aiPlotDirectionX, aiPlotDirectionX, int);
	COPY(m_aiPlotDirectionY, aiPlotDirectionY, int);
	COPY(m_aiPlotCardinalDirectionX, aiPlotCardinalDirectionX, int);
	COPY(m_aiPlotCardinalDirectionY, aiPlotCardinalDirectionY, int);
	COPY(m_aiCityPlotX, aiCityPlotX, int);
	COPY(m_aiCityPlotY, aiCityPlotY, int);
	COPY(m_aiCityPlotPriority, aiCityPlotPriority, int);
	COPY(m_aeTurnLeftDirection, aeTurnLeftDirection, DirectionTypes);
	COPY(m_aeTurnRightDirection, aeTurnRightDirection, DirectionTypes);
	memcpy(m_aaiXYCityPlot, aaiXYCityPlot, sizeof(m_aaiXYCityPlot));
	memcpy(m_aaeXYDirection, aaeXYDirection,sizeof(m_aaeXYDirection));
}

//
// free
//
void CvGlobals::uninit()
{
	//
	// See also CvXMLLoadUtilityInit.cpp::CleanUpGlobalVariables()
	//
	SAFE_DELETE_ARRAY(m_aiPlotDirectionX);
	SAFE_DELETE_ARRAY(m_aiPlotDirectionY);
	SAFE_DELETE_ARRAY(m_aiPlotCardinalDirectionX);
	SAFE_DELETE_ARRAY(m_aiPlotCardinalDirectionY);
	SAFE_DELETE_ARRAY(m_aiCityPlotX);
	SAFE_DELETE_ARRAY(m_aiCityPlotY);
	SAFE_DELETE_ARRAY(m_aiCityPlotPriority);
	SAFE_DELETE_ARRAY(m_aeTurnLeftDirection);
	SAFE_DELETE_ARRAY(m_aeTurnRightDirection);

	SAFE_DELETE(m_game);
	SAFE_DELETE(m_map);

	CvPlayerAI::freeStatics();
	CvTeamAI::freeStatics();

	SAFE_DELETE(m_asyncRand);
	SAFE_DELETE(m_initCore);
	SAFE_DELETE(m_loadedInitCore);
	SAFE_DELETE(m_iniInitCore);
	gDLL->uninitGlobals();	// free globals allocated outside the dll
	SAFE_DELETE(m_VarSystem);

	// already deleted outside of the dll, set to null for safety
	m_messageQueue=NULL;
	m_hotJoinMsgQueue=NULL;
	m_messageControl=NULL;
	m_setupData=NULL;
	m_messageCodes=NULL;
	m_dropMgr=NULL;
	m_portal=NULL;
	m_statsReporter=NULL;
	m_interface=NULL;
	m_diplomacyScreen=NULL;
	m_mpDiplomacyScreen=NULL;
	m_pathFinder=NULL;
	m_interfacePathFinder=NULL;
	m_stepFinder=NULL;
	m_routeFinder=NULL;
	m_borderFinder=NULL;
	m_areaFinder=NULL;
	m_plotGroupFinder=NULL;
	m_stepUnitFinder=NULL;

	delete m_TimeMeasure;
/*************************************************************************************************/
/**	ADDON (WINAMP) merged Sephi																	**/
/*************************************************************************************************/
	delete m_Winamp;
/*************************************************************************************************/
/**	ADDON (WINAMP)						END														**/
/*************************************************************************************************/

	m_typesMap.clear();
	m_aInfoVectors.clear();
}

void CvGlobals::clearTypesMap()
{
	m_typesMap.clear();
	if (m_VarSystem)
	{
		m_VarSystem->UnInit();
	}
}

//Method to convert a string into a religion type.  Initially needed due to a load order issue in the tech infos xml where the religion types are referenced in the tech infos before they are populated.
int CvGlobals::convertReligionTypes(const char* religionString) {
	
	if (strcmp(religionString, "RELIGION_FELLOWSHIP_OF_LEAVES") == 0) return RELIGION_FELLOWSHIP_OF_LEAVES;
	if (strcmp(religionString, "RELIGION_THE_ORDER") == 0) return RELIGION_THE_ORDER;
	if (strcmp(religionString, "RELIGION_OCTOPUS_OVERLORDS") == 0) return RELIGION_OCTOPUS_OVERLORDS;
	if (strcmp(religionString, "RELIGION_RUNES_OF_KILMORPH") == 0) return RELIGION_RUNES_OF_KILMORPH;
	if (strcmp(religionString, "RELIGION_THE_ASHEN_VEIL") == 0) return RELIGION_THE_ASHEN_VEIL;
	if (strcmp(religionString, "RELIGION_THE_EMPYREAN") == 0) return RELIGION_THE_EMPYREAN;
	if (strcmp(religionString, "RELIGION_COUNCIL_OF_ESUS") == 0) return RELIGION_COUNCIL_OF_ESUS;
					
	return NO_RELIGION;
}

CvDiplomacyScreen* CvGlobals::getDiplomacyScreen()
{
	return m_diplomacyScreen;
}

CMPDiplomacyScreen* CvGlobals::getMPDiplomacyScreen()
{
	return m_mpDiplomacyScreen;
}

CvMessageCodeTranslator& CvGlobals::getMessageCodes()
{
	return *m_messageCodes;
}

FMPIManager*& CvGlobals::getFMPMgrPtr()
{
	return m_pFMPMgr;
}

CvPortal& CvGlobals::getPortal()
{
	return *m_portal;
}

CvSetupData& CvGlobals::getSetupData()
{
	return *m_setupData;
}

CvInitCore& CvGlobals::getInitCore()
{
	return *m_initCore;
}

CvInitCore& CvGlobals::getLoadedInitCore()
{
	return *m_loadedInitCore;
}

CvInitCore& CvGlobals::getIniInitCore()
{
	return *m_iniInitCore;
}

CvStatsReporter& CvGlobals::getStatsReporter()
{
	return *m_statsReporter;
}

CvStatsReporter* CvGlobals::getStatsReporterPtr()
{
	return m_statsReporter;
}

CvInterface& CvGlobals::getInterface()
{
	return *m_interface;
}

CvInterface* CvGlobals::getInterfacePtr()
{
	return m_interface;
}

CvRandom& CvGlobals::getASyncRand()
{
	return *m_asyncRand;
}

CMessageQueue& CvGlobals::getMessageQueue()
{
	return *m_messageQueue;
}

CMessageQueue& CvGlobals::getHotMessageQueue()
{
	return *m_hotJoinMsgQueue;
}

CMessageControl& CvGlobals::getMessageControl()
{
	return *m_messageControl;
}

CvDropMgr& CvGlobals::getDropMgr()
{
	return *m_dropMgr;
}

FAStar& CvGlobals::getPathFinder()
{
	return *m_pathFinder;
}

FAStar& CvGlobals::getInterfacePathFinder()
{
	return *m_interfacePathFinder;
}

FAStar& CvGlobals::getStepFinder()
{
	return *m_stepFinder;
}

FAStar& CvGlobals::getRouteFinder()
{
	return *m_routeFinder;
}

FAStar& CvGlobals::getBorderFinder()
{
	return *m_borderFinder;
}

FAStar& CvGlobals::getAreaFinder()
{
	return *m_areaFinder;
}

FAStar& CvGlobals::getPlotGroupFinder()
{
	return *m_plotGroupFinder;
}
FAStar& CvGlobals::getStepUnitFinder()
{
	return *m_stepUnitFinder;
}

NiPoint3& CvGlobals::getPt3Origin()
{
	return m_pt3Origin;
}

std::vector<CvInterfaceModeInfo*>& CvGlobals::getInterfaceModeInfo()		// For Moose - XML Load Util and CvInfos
{
	return m_paInterfaceModeInfo;
}

CvInterfaceModeInfo& CvGlobals::getInterfaceModeInfo(InterfaceModeTypes e)
{
	FAssert(e > -1);
	FAssert(e < NUM_INTERFACEMODE_TYPES);
	return *(m_paInterfaceModeInfo[e]);
}

NiPoint3& CvGlobals::getPt3CameraDir()
{
	return m_pt3CameraDir;
}

bool& CvGlobals::getLogging()
{
	return m_bLogging;
}

bool& CvGlobals::getRandLogging()
{
	return m_bRandLogging;
}

bool& CvGlobals::getSynchLogging()
{
	return m_bSynchLogging;
}

bool& CvGlobals::overwriteLogs()
{
	return m_bOverwriteLogs;
}

int* CvGlobals::getPlotDirectionX()
{
	return m_aiPlotDirectionX;
}

int* CvGlobals::getPlotDirectionY()
{
	return m_aiPlotDirectionY;
}

int* CvGlobals::getPlotCardinalDirectionX()
{
	return m_aiPlotCardinalDirectionX;
}

int* CvGlobals::getPlotCardinalDirectionY()
{
	return m_aiPlotCardinalDirectionY;
}

int* CvGlobals::getCityPlotX()
{
	return m_aiCityPlotX;
}

int* CvGlobals::getCityPlotY()
{
	return m_aiCityPlotY;
}

int* CvGlobals::getCityPlotPriority()
{
	return m_aiCityPlotPriority;
}

int CvGlobals::getXYCityPlot(int i, int j)
{
	FAssertMsg(i < CITY_PLOTS_DIAMETER, "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	FAssertMsg(j < CITY_PLOTS_DIAMETER, "Index out of bounds");
	FAssertMsg(j > -1, "Index out of bounds");
	return m_aaiXYCityPlot[i][j];
}

DirectionTypes* CvGlobals::getTurnLeftDirection()
{
	return m_aeTurnLeftDirection;
}

DirectionTypes CvGlobals::getTurnLeftDirection(int i)
{
	FAssertMsg(i < NUM_DIRECTION_TYPES, "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_aeTurnLeftDirection[i];
}

DirectionTypes* CvGlobals::getTurnRightDirection()
{
	return m_aeTurnRightDirection;
}

DirectionTypes CvGlobals::getTurnRightDirection(int i)
{
	FAssertMsg(i < NUM_DIRECTION_TYPES, "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_aeTurnRightDirection[i];
}

DirectionTypes CvGlobals::getXYDirection(int i, int j)
{
	FAssertMsg(i < DIRECTION_DIAMETER, "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	FAssertMsg(j < DIRECTION_DIAMETER, "Index out of bounds");
	FAssertMsg(j > -1, "Index out of bounds");
	return m_aaeXYDirection[i][j];
}

int CvGlobals::getNumWorldInfos()
{
	return (int)m_paWorldInfo.size();
}

std::vector<CvWorldInfo*>& CvGlobals::getWorldInfo()
{
	return m_paWorldInfo;
}

CvWorldInfo& CvGlobals::getWorldInfo(WorldSizeTypes e)
{
	FAssert(e > -1);
	FAssert(e < GC.getNumWorldInfos());
	return *(m_paWorldInfo[e]);
}

/////////////////////////////////////////////
// CLIMATE
/////////////////////////////////////////////

int CvGlobals::getNumClimateInfos()
{
	return (int)m_paClimateInfo.size();
}

std::vector<CvClimateInfo*>& CvGlobals::getClimateInfo()
{
	return m_paClimateInfo;
}

CvClimateInfo& CvGlobals::getClimateInfo(ClimateTypes e)
{
	FAssert(e > -1);
	FAssert(e < GC.getNumClimateInfos());
	return *(m_paClimateInfo[e]);
}

/////////////////////////////////////////////
// SEALEVEL
/////////////////////////////////////////////

int CvGlobals::getNumSeaLevelInfos()
{
	return (int)m_paSeaLevelInfo.size();
}

std::vector<CvSeaLevelInfo*>& CvGlobals::getSeaLevelInfo()
{
	return m_paSeaLevelInfo;
}

CvSeaLevelInfo& CvGlobals::getSeaLevelInfo(SeaLevelTypes e)
{
	FAssert(e > -1);
	FAssert(e < GC.getNumSeaLevelInfos());
	return *(m_paSeaLevelInfo[e]);
}

int CvGlobals::getNumHints()
{
	return (int)m_paHints.size();
}

std::vector<CvInfoBase*>& CvGlobals::getHints()
{
	return m_paHints;
}

CvInfoBase& CvGlobals::getHints(int i)
{
	return *(m_paHints[i]);
}

int CvGlobals::getNumMainMenus()
{
	return (int)m_paMainMenus.size();
}

std::vector<CvMainMenuInfo*>& CvGlobals::getMainMenus()
{
	return m_paMainMenus;
}

CvMainMenuInfo& CvGlobals::getMainMenus(int i)
{
	if (i >= getNumMainMenus())
	{
		return *(m_paMainMenus[0]);
	}

	return *(m_paMainMenus[i]);
}

/*************************************************************************************************/
/**	TrueModular								05/26/09	Written:WoC Team	Imported: Xienwolf	**/
/**																								**/
/**	Dependency Data is only needed during XML loading, but isn't easily passed between functions**/
/**	So rather than trying to save all of this data on each and every loaded type, it is saved	**/
/**	As a state of the entire DLL.  This can only be accomplished because it is needed at a very	**/
/**	specific time and NOTHING else will happen during that time to intefere with these values	**/
/**																								**/
/**	Properly links Modular modifications to previous elements, and allows partial overwriting	**/
/*************************************************************************************************/
// Python Modular Loading
int CvGlobals::getNumPythonModulesInfos()
{
	return (int)m_paPythonModulesInfo.size();
}

std::vector<CvPythonModulesInfo*>& CvGlobals::getPythonModulesInfo()
{
	return m_paPythonModulesInfo;
}

CvPythonModulesInfo& CvGlobals::getPythonModulesInfo(int iIndex)
{
	FAssertMsg(iIndex < GC.getNumPythonModulesInfos(), "Index out of bounds");
	FAssertMsg(iIndex > -1, "Index out of bounds");
	return *(m_paPythonModulesInfo[iIndex]);
}

// MLF loading
void CvGlobals::resetModLoadControlVector()
{
	m_paModLoadControlVector.clear();
}

int CvGlobals::getModLoadControlVectorSize()
{
	return (int)m_paModLoadControlVector.size();
}

void CvGlobals::setModLoadControlVector(const char* szModule)
{
	m_paModLoadControlVector.push_back(szModule);
}

CvString CvGlobals::getModLoadControlVector(int i)
{
	return (CvString)m_paModLoadControlVector.at(i);
}

int CvGlobals::getTotalNumModules()
{
	return m_iTotalNumModules;
}

void CvGlobals::setTotalNumModules()
{
	m_iTotalNumModules++;
}

int CvGlobals::getNumModLoadControlInfos()
{
	return (int)m_paModLoadControls.size();
}

std::vector<CvModLoadControlInfo*>& CvGlobals::getModLoadControlInfos()
{
	return m_paModLoadControls;
}

CvModLoadControlInfo& CvGlobals::getModLoadControlInfos(int iIndex)
{
	FAssertMsg(iIndex < getNumModLoadControlInfos(), "Index out of bounds");
	FAssertMsg(iIndex > -1, "Index out of bounds");
	return *(m_paModLoadControls[iIndex]);
}
// Modular loading Dependencies
void CvGlobals::resetDependencies()
{
	m_bAnyDependency = false;
	m_bTypeDependency = false;
	m_paszAndDependencyTypes.clear();
	m_paszOrDependencyTypes.clear();
}

bool CvGlobals::isAnyDependency()
{
	return m_bAnyDependency;
}

void CvGlobals::setAnyDependency(bool bAnyDependency)
{
	m_bAnyDependency = bAnyDependency;
}

bool& CvGlobals::getTypeDependency()
{
	return m_bTypeDependency;
}

void CvGlobals::setTypeDependency(bool newValue)
{
	m_bTypeDependency = newValue;
}

const int CvGlobals::getAndNumDependencyTypes() const
{
	return m_paszAndDependencyTypes.size();
}

void CvGlobals::setAndDependencyTypes(const char* szDependencyTypes)
{
	m_paszAndDependencyTypes.push_back(szDependencyTypes);
}

const CvString CvGlobals::getAndDependencyTypes(int i) const
{
	FAssertMsg(i < GC.getAndNumDependencyTypes(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_paszAndDependencyTypes.at(i);
}

const int CvGlobals::getOrNumDependencyTypes() const
{
	return m_paszOrDependencyTypes.size();
}

void CvGlobals::setOrDependencyTypes(const char* szDependencyTypes)
{
	m_paszOrDependencyTypes.push_back(szDependencyTypes);
}

const CvString CvGlobals::getOrDependencyTypes(int i) const
{
	FAssertMsg(i < GC.getOrNumDependencyTypes(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_paszOrDependencyTypes.at(i);
}

void CvGlobals::setModDir(const char* szModDir)
{
	m_cszModDir = szModDir;
}

std::string CvGlobals::getModDir()
{
	return m_cszModDir;
}
/*************************************************************************************************/
/**	TrueModular								END													**/
/*************************************************************************************************/
int CvGlobals::getNumColorInfos()
{
	return (int)m_paColorInfo.size();
}

std::vector<CvColorInfo*>& CvGlobals::getColorInfo()
{
	return m_paColorInfo;
}

CvColorInfo& CvGlobals::getColorInfo(ColorTypes e)
{
	FAssert(e > -1);
	FAssert(e < GC.getNumColorInfos());
	return *(m_paColorInfo[e]);
}


int CvGlobals::getNumPlayerColorInfos()
{
	return (int)m_paPlayerColorInfo.size();
}

std::vector<CvPlayerColorInfo*>& CvGlobals::getPlayerColorInfo()
{
	return m_paPlayerColorInfo;
}

CvPlayerColorInfo& CvGlobals::getPlayerColorInfo(PlayerColorTypes e)
{
	FAssert(e > -1);
	FAssert(e < GC.getNumPlayerColorInfos());
	return *(m_paPlayerColorInfo[e]);
}

int CvGlobals::getNumAdvisorInfos()
{
	return (int)m_paAdvisorInfo.size();
}

std::vector<CvAdvisorInfo*>& CvGlobals::getAdvisorInfo()
{
	return m_paAdvisorInfo;
}

CvAdvisorInfo& CvGlobals::getAdvisorInfo(AdvisorTypes e)
{
	FAssert(e > -1);
	FAssert(e < GC.getNumAdvisorInfos());
	return *(m_paAdvisorInfo[e]);
}

int CvGlobals::getNumRouteModelInfos()
{
	return (int)m_paRouteModelInfo.size();
}

std::vector<CvRouteModelInfo*>& CvGlobals::getRouteModelInfo()
{
	return m_paRouteModelInfo;
}

CvRouteModelInfo& CvGlobals::getRouteModelInfo(int i)
{
	FAssert(i > -1);
	FAssert(i < GC.getNumRouteModelInfos());
	return *(m_paRouteModelInfo[i]);
}

int CvGlobals::getNumRiverInfos()
{
	return (int)m_paRiverInfo.size();
}

std::vector<CvRiverInfo*>& CvGlobals::getRiverInfo()
{
	return m_paRiverInfo;
}

CvRiverInfo& CvGlobals::getRiverInfo(RiverTypes e)
{
	FAssert(e > -1);
	FAssert(e < GC.getNumRiverInfos());
	return *(m_paRiverInfo[e]);
}

int CvGlobals::getNumRiverModelInfos()
{
	return (int)m_paRiverModelInfo.size();
}

std::vector<CvRiverModelInfo*>& CvGlobals::getRiverModelInfo()
{
	return m_paRiverModelInfo;
}

CvRiverModelInfo& CvGlobals::getRiverModelInfo(int i)
{
	FAssert(i > -1);
	FAssert(i < GC.getNumRiverModelInfos());
	return *(m_paRiverModelInfo[i]);
}

int CvGlobals::getNumWaterPlaneInfos()
{
	return (int)m_paWaterPlaneInfo.size();
}

std::vector<CvWaterPlaneInfo*>& CvGlobals::getWaterPlaneInfo()		// For Moose - CvDecal and CvWater
{
	return m_paWaterPlaneInfo;
}

CvWaterPlaneInfo& CvGlobals::getWaterPlaneInfo(int i)
{
	FAssert(i > -1);
	FAssert(i < GC.getNumWaterPlaneInfos());
	return *(m_paWaterPlaneInfo[i]);
}

int CvGlobals::getNumTerrainPlaneInfos()
{
	return (int)m_paTerrainPlaneInfo.size();
}

std::vector<CvTerrainPlaneInfo*>& CvGlobals::getTerrainPlaneInfo()
{
	return m_paTerrainPlaneInfo;
}

CvTerrainPlaneInfo& CvGlobals::getTerrainPlaneInfo(int i)
{
	FAssert(i > -1);
	FAssert(i < GC.getNumTerrainPlaneInfos());
	return *(m_paTerrainPlaneInfo[i]);
}

int CvGlobals::getNumCameraOverlayInfos()
{
	return (int)m_paCameraOverlayInfo.size();
}

std::vector<CvCameraOverlayInfo*>& CvGlobals::getCameraOverlayInfo()
{
	return m_paCameraOverlayInfo;
}

CvCameraOverlayInfo& CvGlobals::getCameraOverlayInfo(int i)
{
	FAssert(i > -1);
	FAssert(i < GC.getNumCameraOverlayInfos());
	return *(m_paCameraOverlayInfo[i]);
}

int CvGlobals::getNumAnimationPathInfos()
{
	return (int)m_paAnimationPathInfo.size();
}

std::vector<CvAnimationPathInfo*>& CvGlobals::getAnimationPathInfo()
{
	return m_paAnimationPathInfo;
}

CvAnimationPathInfo& CvGlobals::getAnimationPathInfo(AnimationPathTypes e)
{
	FAssert(e > -1);
	FAssert(e < GC.getNumAnimationPathInfos());
	return *(m_paAnimationPathInfo[e]);
}

int CvGlobals::getNumAnimationCategoryInfos()
{
	return (int)m_paAnimationCategoryInfo.size();
}

std::vector<CvAnimationCategoryInfo*>& CvGlobals::getAnimationCategoryInfo()
{
	return m_paAnimationCategoryInfo;
}

CvAnimationCategoryInfo& CvGlobals::getAnimationCategoryInfo(AnimationCategoryTypes e)
{
	FAssert(e > -1);
	FAssert(e < GC.getNumAnimationCategoryInfos());
	return *(m_paAnimationCategoryInfo[e]);
}

int CvGlobals::getNumEntityEventInfos()
{
	return (int)m_paEntityEventInfo.size();
}

std::vector<CvEntityEventInfo*>& CvGlobals::getEntityEventInfo()
{
	return m_paEntityEventInfo;
}

CvEntityEventInfo& CvGlobals::getEntityEventInfo(EntityEventTypes e)
{
	FAssert( e > -1 );
	FAssert( e < GC.getNumEntityEventInfos() );
	return *(m_paEntityEventInfo[e]);
}

int CvGlobals::getNumEffectInfos()
{
	return (int)m_paEffectInfo.size();
}

std::vector<CvEffectInfo*>& CvGlobals::getEffectInfo()
{
	return m_paEffectInfo;
}

CvEffectInfo& CvGlobals::getEffectInfo(int i)
{
	FAssert(i > -1);
	FAssert(i < GC.getNumEffectInfos());
	return *(m_paEffectInfo[i]);
}


int CvGlobals::getNumAttachableInfos()
{
	return (int)m_paAttachableInfo.size();
}

std::vector<CvAttachableInfo*>& CvGlobals::getAttachableInfo()
{
	return m_paAttachableInfo;
}

CvAttachableInfo& CvGlobals::getAttachableInfo(int i)
{
	FAssert(i > -1);
	FAssert(i < GC.getNumAttachableInfos());
	return *(m_paAttachableInfo[i]);
}

int CvGlobals::getNumCameraInfos()
{
	return (int)m_paCameraInfo.size();
}

std::vector<CvCameraInfo*>& CvGlobals::getCameraInfo()
{
	return m_paCameraInfo;
}

CvCameraInfo& CvGlobals::getCameraInfo(CameraAnimationTypes eCameraAnimationNum)
{
	return *(m_paCameraInfo[eCameraAnimationNum]);
}

int CvGlobals::getNumUnitFormationInfos()
{
	return (int)m_paUnitFormationInfo.size();
}

std::vector<CvUnitFormationInfo*>& CvGlobals::getUnitFormationInfo()		// For Moose - CvUnitEntity
{
	return m_paUnitFormationInfo;
}

CvUnitFormationInfo& CvGlobals::getUnitFormationInfo(int i)
{
	FAssert(i > -1);
	FAssert(i < GC.getNumUnitFormationInfos());
	return *(m_paUnitFormationInfo[i]);
}

// TEXT
int CvGlobals::getNumGameTextXML()
{
	return (int)m_paGameTextXML.size();
}

std::vector<CvGameText*>& CvGlobals::getGameTextXML()
{
	return m_paGameTextXML;
}

// Landscape INFOS
int CvGlobals::getNumLandscapeInfos()
{
	return (int)m_paLandscapeInfo.size();
}

std::vector<CvLandscapeInfo*>& CvGlobals::getLandscapeInfo()
{
	return m_paLandscapeInfo;
}

CvLandscapeInfo& CvGlobals::getLandscapeInfo(int iIndex)
{
	FAssert(iIndex > -1);
	FAssert(iIndex < GC.getNumLandscapeInfos());
	return *(m_paLandscapeInfo[iIndex]);
}

int CvGlobals::getActiveLandscapeID()
{
	return m_iActiveLandscapeID;
}

void CvGlobals::setActiveLandscapeID(int iLandscapeID)
{
	m_iActiveLandscapeID = iLandscapeID;
}


int CvGlobals::getNumTerrainInfos()
{
	return (int)m_paTerrainInfo.size();
}

std::vector<CvTerrainInfo*>& CvGlobals::getTerrainInfo()		// For Moose - XML Load Util, CvInfos, CvTerrainTypeWBPalette
{
	return m_paTerrainInfo;
}

CvTerrainInfo& CvGlobals::getTerrainInfo(TerrainTypes eTerrainNum)
{
	FAssert(eTerrainNum > -1);
	FAssert(eTerrainNum < GC.getNumTerrainInfos());
	return *(m_paTerrainInfo[eTerrainNum]);
}

int CvGlobals::getNumBonusClassInfos()
{
	return (int)m_paBonusClassInfo.size();
}

std::vector<CvBonusClassInfo*>& CvGlobals::getBonusClassInfo()	// For Moose - XML Load Util, CvInfos
{
	return m_paBonusClassInfo;
}

CvBonusClassInfo& CvGlobals::getBonusClassInfo(BonusClassTypes eBonusNum)
{
	FAssert(eBonusNum > -1);
	FAssert(eBonusNum < GC.getNumBonusClassInfos());
	return *(m_paBonusClassInfo[eBonusNum]);
}


int CvGlobals::getNumBonusInfos()
{
	return (int)m_paBonusInfo.size();
}

std::vector<CvBonusInfo*>& CvGlobals::getBonusInfo()	// For Moose - XML Load Util, CvInfos
{
	return m_paBonusInfo;
}

CvBonusInfo& CvGlobals::getBonusInfo(BonusTypes eBonusNum)
{
	FAssert(eBonusNum > -1);
	FAssert(eBonusNum < GC.getNumBonusInfos());
	return *(m_paBonusInfo[eBonusNum]);
}

int CvGlobals::getNumFeatureInfos()
{
	return (int)m_paFeatureInfo.size();
}

std::vector<CvFeatureInfo*>& CvGlobals::getFeatureInfo()	// For Moose - XML Load Util, CvInfos
{
	return m_paFeatureInfo;
}

CvFeatureInfo& CvGlobals::getFeatureInfo(FeatureTypes eFeatureNum)
{
	FAssert(eFeatureNum > -1);
	FAssert(eFeatureNum < GC.getNumFeatureInfos());
	return *(m_paFeatureInfo[eFeatureNum]);
}

int& CvGlobals::getNumPlayableCivilizationInfos()
{
	return m_iNumPlayableCivilizationInfos;
}

int& CvGlobals::getNumAIPlayableCivilizationInfos()
{
	return m_iNumAIPlayableCivilizationInfos;
}

int CvGlobals::getNumCivilizationInfos()
{
	return (int)m_paCivilizationInfo.size();
}

std::vector<CvCivilizationInfo*>& CvGlobals::getCivilizationInfo()	// For Moose - XML Load Util, CvInfos
{
	return m_paCivilizationInfo;
}

CvCivilizationInfo& CvGlobals::getCivilizationInfo(CivilizationTypes eCivilizationNum)
{
	FAssert(eCivilizationNum > -1);
	FAssert(eCivilizationNum < GC.getNumCivilizationInfos());
	return *(m_paCivilizationInfo[eCivilizationNum]);
}


int CvGlobals::getNumLeaderHeadInfos()
{
	return (int)m_paLeaderHeadInfo.size();
}

std::vector<CvLeaderHeadInfo*>& CvGlobals::getLeaderHeadInfo()	// For Moose - XML Load Util, CvInfos
{
	return m_paLeaderHeadInfo;
}

CvLeaderHeadInfo& CvGlobals::getLeaderHeadInfo(LeaderHeadTypes eLeaderHeadNum)
{
	FAssert(eLeaderHeadNum > -1);
	FAssert(eLeaderHeadNum < GC.getNumLeaderHeadInfos());
	return *(m_paLeaderHeadInfo[eLeaderHeadNum]);
}


int CvGlobals::getNumTraitInfos()
{
	return (int)m_paTraitInfo.size();
}

std::vector<CvTraitInfo*>& CvGlobals::getTraitInfo()	// For Moose - XML Load Util, CvInfos
{
	return m_paTraitInfo;
}

CvTraitInfo& CvGlobals::getTraitInfo(TraitTypes eTraitNum)
{
	FAssert(eTraitNum > -1);
	FAssert(eTraitNum < GC.getNumTraitInfos());
	return *(m_paTraitInfo[eTraitNum]);
}


int CvGlobals::getNumCursorInfos()
{
	return (int)m_paCursorInfo.size();
}

std::vector<CvCursorInfo*>& CvGlobals::getCursorInfo()	// For Moose - XML Load Util, CvInfos
{
	return m_paCursorInfo;
}

CvCursorInfo& CvGlobals::getCursorInfo(CursorTypes eCursorNum)
{
	FAssert(eCursorNum > -1);
	FAssert(eCursorNum < GC.getNumCursorInfos());
	return *(m_paCursorInfo[eCursorNum]);
}

int CvGlobals::getNumThroneRoomCameras()
{
	return (int)m_paThroneRoomCamera.size();
}

std::vector<CvThroneRoomCamera*>& CvGlobals::getThroneRoomCamera()	// For Moose - XML Load Util, CvInfos
{
	return m_paThroneRoomCamera;
}

CvThroneRoomCamera& CvGlobals::getThroneRoomCamera(int iIndex)
{
	FAssert(iIndex > -1);
	FAssert(iIndex < GC.getNumThroneRoomCameras());
	return *(m_paThroneRoomCamera[iIndex]);
}

int CvGlobals::getNumThroneRoomInfos()
{
	return (int)m_paThroneRoomInfo.size();
}

std::vector<CvThroneRoomInfo*>& CvGlobals::getThroneRoomInfo()	// For Moose - XML Load Util, CvInfos
{
	return m_paThroneRoomInfo;
}

CvThroneRoomInfo& CvGlobals::getThroneRoomInfo(int iIndex)
{
	FAssert(iIndex > -1);
	FAssert(iIndex < GC.getNumThroneRoomInfos());
	return *(m_paThroneRoomInfo[iIndex]);
}

int CvGlobals::getNumThroneRoomStyleInfos()
{
	return (int)m_paThroneRoomStyleInfo.size();
}

std::vector<CvThroneRoomStyleInfo*>& CvGlobals::getThroneRoomStyleInfo()	// For Moose - XML Load Util, CvInfos
{
	return m_paThroneRoomStyleInfo;
}

CvThroneRoomStyleInfo& CvGlobals::getThroneRoomStyleInfo(int iIndex)
{
	FAssert(iIndex > -1);
	FAssert(iIndex < GC.getNumThroneRoomStyleInfos());
	return *(m_paThroneRoomStyleInfo[iIndex]);
}

int CvGlobals::getNumSlideShowInfos()
{
	return (int)m_paSlideShowInfo.size();
}

std::vector<CvSlideShowInfo*>& CvGlobals::getSlideShowInfo()	// For Moose - XML Load Util, CvInfos
{
	return m_paSlideShowInfo;
}

CvSlideShowInfo& CvGlobals::getSlideShowInfo(int iIndex)
{
	FAssert(iIndex > -1);
	FAssert(iIndex < GC.getNumSlideShowInfos());
	return *(m_paSlideShowInfo[iIndex]);
}

int CvGlobals::getNumSlideShowRandomInfos()
{
	return (int)m_paSlideShowRandomInfo.size();
}

std::vector<CvSlideShowRandomInfo*>& CvGlobals::getSlideShowRandomInfo()	// For Moose - XML Load Util, CvInfos
{
	return m_paSlideShowRandomInfo;
}

CvSlideShowRandomInfo& CvGlobals::getSlideShowRandomInfo(int iIndex)
{
	FAssert(iIndex > -1);
	FAssert(iIndex < GC.getNumSlideShowRandomInfos());
	return *(m_paSlideShowRandomInfo[iIndex]);
}

int CvGlobals::getNumWorldPickerInfos()
{
	return (int)m_paWorldPickerInfo.size();
}

std::vector<CvWorldPickerInfo*>& CvGlobals::getWorldPickerInfo()	// For Moose - XML Load Util, CvInfos
{
	return m_paWorldPickerInfo;
}

CvWorldPickerInfo& CvGlobals::getWorldPickerInfo(int iIndex)
{
	FAssert(iIndex > -1);
	FAssert(iIndex < GC.getNumWorldPickerInfos());
	return *(m_paWorldPickerInfo[iIndex]);
}

int CvGlobals::getNumSpaceShipInfos()
{
	return (int)m_paSpaceShipInfo.size();
}

std::vector<CvSpaceShipInfo*>& CvGlobals::getSpaceShipInfo()	// For Moose - XML Load Util, CvInfos
{
	return m_paSpaceShipInfo;
}

CvSpaceShipInfo& CvGlobals::getSpaceShipInfo(int iIndex)
{
	FAssert(iIndex > -1);
	FAssert(iIndex < GC.getNumSpaceShipInfos());
	return *(m_paSpaceShipInfo[iIndex]);
}

int CvGlobals::getNumUnitInfos()
{
	return (int)m_paUnitInfo.size();
}

std::vector<CvUnitInfo*>& CvGlobals::getUnitInfo()	// For Moose - XML Load Util, CvInfos
{
	return m_paUnitInfo;
}

CvUnitInfo& CvGlobals::getUnitInfo(UnitTypes eUnitNum)
{
	FAssert(eUnitNum > -1);
	FAssert(eUnitNum < GC.getNumUnitInfos());
	return *(m_paUnitInfo[eUnitNum]);
}

int CvGlobals::getNumSpecialUnitInfos()
{
	return (int)m_paSpecialUnitInfo.size();
}

std::vector<CvSpecialUnitInfo*>& CvGlobals::getSpecialUnitInfo()	// For Moose - XML Load Util, CvInfos
{
	return m_paSpecialUnitInfo;
}

CvSpecialUnitInfo& CvGlobals::getSpecialUnitInfo(SpecialUnitTypes eSpecialUnitNum)
{
	FAssert(eSpecialUnitNum > -1);
	FAssert(eSpecialUnitNum < GC.getNumSpecialUnitInfos());
	return *(m_paSpecialUnitInfo[eSpecialUnitNum]);
}


int CvGlobals::getNumConceptInfos()
{
	return (int)m_paConceptInfo.size();
}

std::vector<CvInfoBase*>& CvGlobals::getConceptInfo()	// For Moose - XML Load Util, CvInfos
{
	return m_paConceptInfo;
}

CvInfoBase& CvGlobals::getConceptInfo(ConceptTypes e)
{
	FAssert(e > -1);
	FAssert(e < GC.getNumConceptInfos());
	return *(m_paConceptInfo[e]);
}


int CvGlobals::getNumNewConceptInfos()
{
	return (int)m_paNewConceptInfo.size();
}

std::vector<CvInfoBase*>& CvGlobals::getNewConceptInfo()	// For Moose - XML Load Util, CvInfos
{
	return m_paNewConceptInfo;
}

CvInfoBase& CvGlobals::getNewConceptInfo(NewConceptTypes e)
{
	FAssert(e > -1);
	FAssert(e < GC.getNumNewConceptInfos());
	return *(m_paNewConceptInfo[e]);
}

/*************************************************************************************************/
/**	ADDON (New Pedia Categories) Sephi                                      					**/
/*************************************************************************************************/
int CvGlobals::getNumWildmanaConceptInfos()
{
	return (int)m_paWildmanaConceptInfo.size();
}

std::vector<CvInfoBase*>& CvGlobals::getWildmanaConceptInfo()
{
	return m_paWildmanaConceptInfo;
}

CvInfoBase& CvGlobals::getWildmanaConceptInfo(WildmanaConceptTypes e)
{
	FAssert(e > -1);
	FAssert(e < GC.getNumWildmanaConceptInfos());
	return *(m_paWildmanaConceptInfo[e]);
}

int CvGlobals::getNumWildmanaGuideInfos()
{
	return (int)m_paWildmanaGuideInfo.size();
}

std::vector<CvInfoBase*>& CvGlobals::getWildmanaGuideInfo()	// For Moose - XML Load Util, CvInfos
{
	return m_paWildmanaGuideInfo;
}

CvInfoBase& CvGlobals::getWildmanaGuideInfo(WildmanaGuideTypes e)
{
	FAssert(e > -1);
	FAssert(e < GC.getNumWildmanaGuideInfos());
	return *(m_paWildmanaGuideInfo[e]);
}
/*************************************************************************************************/
/**	END	                                        												**/
/*************************************************************************************************/
int CvGlobals::getNumCityTabInfos()
{
	return (int)m_paCityTabInfo.size();
}

std::vector<CvInfoBase*>& CvGlobals::getCityTabInfo()	// For Moose - XML Load Util, CvInfos
{
	return m_paCityTabInfo;
}

CvInfoBase& CvGlobals::getCityTabInfo(CityTabTypes e)
{
	FAssert(e > -1);
	FAssert(e < GC.getNumCityTabInfos());
	return *(m_paCityTabInfo[e]);
}


int CvGlobals::getNumCalendarInfos()
{
	return (int)m_paCalendarInfo.size();
}

std::vector<CvInfoBase*>& CvGlobals::getCalendarInfo()
{
	return m_paCalendarInfo;
}

CvInfoBase& CvGlobals::getCalendarInfo(CalendarTypes e)
{
	FAssert(e > -1);
	FAssert(e < GC.getNumCalendarInfos());
	return *(m_paCalendarInfo[e]);
}


int CvGlobals::getNumSeasonInfos()
{
	return (int)m_paSeasonInfo.size();
}

std::vector<CvInfoBase*>& CvGlobals::getSeasonInfo()	// For Moose - XML Load Util, CvInfos
{
	return m_paSeasonInfo;
}

CvInfoBase& CvGlobals::getSeasonInfo(SeasonTypes e)
{
	FAssert(e > -1);
	FAssert(e < GC.getNumSeasonInfos());
	return *(m_paSeasonInfo[e]);
}


int CvGlobals::getNumMonthInfos()
{
	return (int)m_paMonthInfo.size();
}

std::vector<CvInfoBase*>& CvGlobals::getMonthInfo()	// For Moose - XML Load Util, CvInfos
{
	return m_paMonthInfo;
}

CvInfoBase& CvGlobals::getMonthInfo(MonthTypes e)
{
	FAssert(e > -1);
	FAssert(e < GC.getNumMonthInfos());
	return *(m_paMonthInfo[e]);
}


int CvGlobals::getNumDenialInfos()
{
	return (int)m_paDenialInfo.size();
}

std::vector<CvInfoBase*>& CvGlobals::getDenialInfo()	// For Moose - XML Load Util, CvInfos
{
	return m_paDenialInfo;
}

CvInfoBase& CvGlobals::getDenialInfo(DenialTypes e)
{
	FAssert(e > -1);
	FAssert(e < GC.getNumDenialInfos());
	return *(m_paDenialInfo[e]);
}


int CvGlobals::getNumInvisibleInfos()
{
	return (int)m_paInvisibleInfo.size();
}

std::vector<CvInfoBase*>& CvGlobals::getInvisibleInfo()	// For Moose - XML Load Util, CvInfos
{
	return m_paInvisibleInfo;
}

CvInfoBase& CvGlobals::getInvisibleInfo(InvisibleTypes e)
{
	FAssert(e > -1);
	FAssert(e < GC.getNumInvisibleInfos());
	return *(m_paInvisibleInfo[e]);
}


int CvGlobals::getNumVoteSourceInfos()
{
	return (int)m_paVoteSourceInfo.size();
}

std::vector<CvVoteSourceInfo*>& CvGlobals::getVoteSourceInfo()	// For Moose - XML Load Util, CvInfos
{
	return m_paVoteSourceInfo;
}

CvVoteSourceInfo& CvGlobals::getVoteSourceInfo(VoteSourceTypes e)
{
	FAssert(e > -1);
	FAssert(e < GC.getNumVoteSourceInfos());
	return *(m_paVoteSourceInfo[e]);
}


int CvGlobals::getNumUnitCombatInfos()
{
	return (int)m_paUnitCombatInfo.size();
}

std::vector<CvUnitCombatInfo*>& CvGlobals::getUnitCombatInfo()
{
	return m_paUnitCombatInfo;
}

CvUnitCombatInfo& CvGlobals::getUnitCombatInfo(UnitCombatTypes e)
{
	FAssert(e > -1);
	FAssert(e < GC.getNumUnitCombatInfos());
	return *(m_paUnitCombatInfo[e]);
}


std::vector<CvInfoBase*>& CvGlobals::getDomainInfo()
{
	return m_paDomainInfo;
}

CvInfoBase& CvGlobals::getDomainInfo(DomainTypes e)
{
	FAssert(e > -1);
	FAssert(e < NUM_DOMAIN_TYPES);
	return *(m_paDomainInfo[e]);
}


std::vector<CvInfoBase*>& CvGlobals::getUnitAIInfo()
{
	return m_paUnitAIInfos;
}

CvInfoBase& CvGlobals::getUnitAIInfo(UnitAITypes eUnitAINum)
{
	FAssert(eUnitAINum >= 0);
	FAssert(eUnitAINum < NUM_UNITAI_TYPES);
	return *(m_paUnitAIInfos[eUnitAINum]);
}

/*************************************************************************************************/
/**	ADDON (Better AI) Sephi                                      					**/
/*************************************************************************************************/
std::vector<CvInfoBase*>& CvGlobals::getAIGroupInfo()
{
	return m_paAIGroupInfos;
}

CvInfoBase& CvGlobals::getAIGroupInfo(AIGroupTypes eAIGroupNum)
{
	FAssert(eAIGroupNum >= 0);
	FAssert(eAIGroupNum < NUM_AIGROUP_TYPES);
	return *(m_paAIGroupInfos[eAIGroupNum]);
}

/*************************************************************************************************/
/**	END	                                        												**/
/*************************************************************************************************/

std::vector<CvInfoBase*>& CvGlobals::getAttitudeInfo()
{
	return m_paAttitudeInfos;
}

CvInfoBase& CvGlobals::getAttitudeInfo(AttitudeTypes eAttitudeNum)
{
	FAssert(eAttitudeNum >= 0);
	FAssert(eAttitudeNum < NUM_ATTITUDE_TYPES);
	return *(m_paAttitudeInfos[eAttitudeNum]);
}


std::vector<CvInfoBase*>& CvGlobals::getMemoryInfo()
{
	return m_paMemoryInfos;
}

CvInfoBase& CvGlobals::getMemoryInfo(MemoryTypes eMemoryNum)
{
	FAssert(eMemoryNum >= 0);
	FAssert(eMemoryNum < NUM_MEMORY_TYPES);
	return *(m_paMemoryInfos[eMemoryNum]);
}


int CvGlobals::getNumGameOptionInfos()
{
	return (int)m_paGameOptionInfos.size();
}

std::vector<CvGameOptionInfo*>& CvGlobals::getGameOptionInfo()
{
	return m_paGameOptionInfos;
}

CvGameOptionInfo& CvGlobals::getGameOptionInfo(GameOptionTypes eGameOptionNum)
{
	FAssert(eGameOptionNum >= 0);
	FAssert(eGameOptionNum < GC.getNumGameOptionInfos());
	return *(m_paGameOptionInfos[eGameOptionNum]);
}

int CvGlobals::getNumMPOptionInfos()
{
	return (int)m_paMPOptionInfos.size();
}

std::vector<CvMPOptionInfo*>& CvGlobals::getMPOptionInfo()
{
	 return m_paMPOptionInfos;
}

CvMPOptionInfo& CvGlobals::getMPOptionInfo(MultiplayerOptionTypes eMPOptionNum)
{
	FAssert(eMPOptionNum >= 0);
	FAssert(eMPOptionNum < GC.getNumMPOptionInfos());
	return *(m_paMPOptionInfos[eMPOptionNum]);
}

int CvGlobals::getNumForceControlInfos()
{
	return (int)m_paForceControlInfos.size();
}

std::vector<CvForceControlInfo*>& CvGlobals::getForceControlInfo()
{
	return m_paForceControlInfos;
}

CvForceControlInfo& CvGlobals::getForceControlInfo(ForceControlTypes eForceControlNum)
{
	FAssert(eForceControlNum >= 0);
	FAssert(eForceControlNum < GC.getNumForceControlInfos());
	return *(m_paForceControlInfos[eForceControlNum]);
}

std::vector<CvPlayerOptionInfo*>& CvGlobals::getPlayerOptionInfo()
{
	return m_paPlayerOptionInfos;
}

CvPlayerOptionInfo& CvGlobals::getPlayerOptionInfo(PlayerOptionTypes ePlayerOptionNum)
{
	FAssert(ePlayerOptionNum >= 0);
	FAssert(ePlayerOptionNum < NUM_PLAYEROPTION_TYPES);
	return *(m_paPlayerOptionInfos[ePlayerOptionNum]);
}

std::vector<CvGraphicOptionInfo*>& CvGlobals::getGraphicOptionInfo()
{
	return m_paGraphicOptionInfos;
}

CvGraphicOptionInfo& CvGlobals::getGraphicOptionInfo(GraphicOptionTypes eGraphicOptionNum)
{
	FAssert(eGraphicOptionNum >= 0);
	FAssert(eGraphicOptionNum < NUM_GRAPHICOPTION_TYPES);
	return *(m_paGraphicOptionInfos[eGraphicOptionNum]);
}


std::vector<CvYieldInfo*>& CvGlobals::getYieldInfo()	// For Moose - XML Load Util
{
	return m_paYieldInfo;
}

CvYieldInfo& CvGlobals::getYieldInfo(YieldTypes eYieldNum)
{
	FAssert(eYieldNum > -1);
	FAssert(eYieldNum < NUM_YIELD_TYPES);
	return *(m_paYieldInfo[eYieldNum]);
}


std::vector<CvCommerceInfo*>& CvGlobals::getCommerceInfo()	// For Moose - XML Load Util
{
	return m_paCommerceInfo;
}

CvCommerceInfo& CvGlobals::getCommerceInfo(CommerceTypes eCommerceNum)
{
	FAssert(eCommerceNum > -1);
	FAssert(eCommerceNum < NUM_COMMERCE_TYPES);
	return *(m_paCommerceInfo[eCommerceNum]);
}

int CvGlobals::getNumRouteInfos()
{
	return (int)m_paRouteInfo.size();
}

std::vector<CvRouteInfo*>& CvGlobals::getRouteInfo()	// For Moose - XML Load Util, CvInfos
{
	return m_paRouteInfo;
}

CvRouteInfo& CvGlobals::getRouteInfo(RouteTypes eRouteNum)
{
	FAssert(eRouteNum > -1);
	FAssert(eRouteNum < GC.getNumRouteInfos());
	return *(m_paRouteInfo[eRouteNum]);
}

int CvGlobals::getNumImprovementInfos()
{
	return (int)m_paImprovementInfo.size();
}

std::vector<CvImprovementInfo*>& CvGlobals::getImprovementInfo()	// For Moose - XML Load Util, CvInfos
{
	return m_paImprovementInfo;
}

CvImprovementInfo& CvGlobals::getImprovementInfo(ImprovementTypes eImprovementNum)
{
	FAssert(eImprovementNum > -1);
	FAssert(eImprovementNum < GC.getNumImprovementInfos());
	return *(m_paImprovementInfo[eImprovementNum]);
}

int CvGlobals::getNumGoodyInfos()
{
	return (int)m_paGoodyInfo.size();
}

std::vector<CvGoodyInfo*>& CvGlobals::getGoodyInfo()	// For Moose - XML Load Util, CvInfos
{
	return m_paGoodyInfo;
}

CvGoodyInfo& CvGlobals::getGoodyInfo(GoodyTypes eGoodyNum)
{
	FAssert(eGoodyNum > -1);
	FAssert(eGoodyNum < GC.getNumGoodyInfos());
	return *(m_paGoodyInfo[eGoodyNum]);
}

int CvGlobals::getNumBuildInfos()
{
	return (int)m_paBuildInfo.size();
}

std::vector<CvBuildInfo*>& CvGlobals::getBuildInfo()	// For Moose - XML Load Util, CvInfos
{
	return m_paBuildInfo;
}

CvBuildInfo& CvGlobals::getBuildInfo(BuildTypes eBuildNum)
{
	FAssert(eBuildNum > -1);
	FAssert(eBuildNum < GC.getNumBuildInfos());
	return *(m_paBuildInfo[eBuildNum]);
}

int CvGlobals::getNumHandicapInfos()
{
	return (int)m_paHandicapInfo.size();
}

std::vector<CvHandicapInfo*>& CvGlobals::getHandicapInfo()	// Do NOT export outside of the DLL	// For Moose - XML Load Util
{
	return m_paHandicapInfo;
}

CvHandicapInfo& CvGlobals::getHandicapInfo(HandicapTypes eHandicapNum)
{
	FAssert(eHandicapNum > -1);
	FAssert(eHandicapNum < GC.getNumHandicapInfos());
	return *(m_paHandicapInfo[eHandicapNum]);
}

int CvGlobals::getNumGameSpeedInfos()
{
	return (int)m_paGameSpeedInfo.size();
}

std::vector<CvGameSpeedInfo*>& CvGlobals::getGameSpeedInfo()	// Do NOT export outside of the DLL	// For Moose - XML Load Util
{
	return m_paGameSpeedInfo;
}

CvGameSpeedInfo& CvGlobals::getGameSpeedInfo(GameSpeedTypes eGameSpeedNum)
{
	FAssert(eGameSpeedNum > -1);
	FAssert(eGameSpeedNum < GC.getNumGameSpeedInfos());
	return *(m_paGameSpeedInfo[eGameSpeedNum]);
}

int CvGlobals::getNumTurnTimerInfos()
{
	return (int)m_paTurnTimerInfo.size();
}

std::vector<CvTurnTimerInfo*>& CvGlobals::getTurnTimerInfo()	// Do NOT export outside of the DLL	// For Moose - XML Load Util
{
	return m_paTurnTimerInfo;
}

CvTurnTimerInfo& CvGlobals::getTurnTimerInfo(TurnTimerTypes eTurnTimerNum)
{
	FAssert(eTurnTimerNum > -1);
	FAssert(eTurnTimerNum < GC.getNumTurnTimerInfos());
	return *(m_paTurnTimerInfo[eTurnTimerNum]);
}

int CvGlobals::getNumProcessInfos()
{
	return (int)m_paProcessInfo.size();
}

std::vector<CvProcessInfo*>& CvGlobals::getProcessInfo()
{
	return m_paProcessInfo;
}

CvProcessInfo& CvGlobals::getProcessInfo(ProcessTypes e)
{
	FAssert(e > -1);
	FAssert(e < GC.getNumProcessInfos());
	return *(m_paProcessInfo[e]);
}

int CvGlobals::getNumVoteInfos()
{
	return (int)m_paVoteInfo.size();
}

std::vector<CvVoteInfo*>& CvGlobals::getVoteInfo()
{
	return m_paVoteInfo;
}

CvVoteInfo& CvGlobals::getVoteInfo(VoteTypes e)
{
	FAssert(e > -1);
	FAssert(e < GC.getNumVoteInfos());
	return *(m_paVoteInfo[e]);
}

int CvGlobals::getNumProjectInfos()
{
	return (int)m_paProjectInfo.size();
}

std::vector<CvProjectInfo*>& CvGlobals::getProjectInfo()
{
	return m_paProjectInfo;
}

CvProjectInfo& CvGlobals::getProjectInfo(ProjectTypes e)
{
	FAssert(e > -1);
	FAssert(e < GC.getNumProjectInfos());
	return *(m_paProjectInfo[e]);
}

int CvGlobals::getNumBuildingClassInfos()
{
	return (int)m_paBuildingClassInfo.size();
}

std::vector<CvBuildingClassInfo*>& CvGlobals::getBuildingClassInfo()	// For Moose - XML Load Util, CvInfos
{
	return m_paBuildingClassInfo;
}

CvBuildingClassInfo& CvGlobals::getBuildingClassInfo(BuildingClassTypes eBuildingClassNum)
{
	FAssert(eBuildingClassNum > -1);
	FAssert(eBuildingClassNum < GC.getNumBuildingClassInfos());
	return *(m_paBuildingClassInfo[eBuildingClassNum]);
}

int CvGlobals::getNumBuildingInfos()
{
	return (int)m_paBuildingInfo.size();
}

std::vector<CvBuildingInfo*>& CvGlobals::getBuildingInfo()	// For Moose - XML Load Util, CvInfos, CvCacheObject
{
	return m_paBuildingInfo;
}

CvBuildingInfo& CvGlobals::getBuildingInfo(BuildingTypes eBuildingNum)
{
	FAssert(eBuildingNum > -1);
	FAssert(eBuildingNum < GC.getNumBuildingInfos());
	return *(m_paBuildingInfo[eBuildingNum]);
}

int CvGlobals::getNumSpecialBuildingInfos()
{
	return (int)m_paSpecialBuildingInfo.size();
}

std::vector<CvSpecialBuildingInfo*>& CvGlobals::getSpecialBuildingInfo()	// For Moose - XML Load Util, CvInfos
{
	return m_paSpecialBuildingInfo;
}

CvSpecialBuildingInfo& CvGlobals::getSpecialBuildingInfo(SpecialBuildingTypes eSpecialBuildingNum)
{
	FAssert(eSpecialBuildingNum > -1);
	FAssert(eSpecialBuildingNum < GC.getNumSpecialBuildingInfos());
	return *(m_paSpecialBuildingInfo[eSpecialBuildingNum]);
}

int CvGlobals::getNumUnitClassInfos()
{
	return (int)m_paUnitClassInfo.size();
}

std::vector<CvUnitClassInfo*>& CvGlobals::getUnitClassInfo()	// For Moose - XML Load Util, CvInfos
{
	return m_paUnitClassInfo;
}

CvUnitClassInfo& CvGlobals::getUnitClassInfo(UnitClassTypes eUnitClassNum)
{
	FAssert(eUnitClassNum > -1);
	FAssert(eUnitClassNum < GC.getNumUnitClassInfos());
	return *(m_paUnitClassInfo[eUnitClassNum]);
}

int CvGlobals::getNumActionInfos()
{
	return (int)m_paActionInfo.size();
}

std::vector<CvActionInfo*>& CvGlobals::getActionInfo()	// For Moose - XML Load Util
{
	return m_paActionInfo;
}

CvActionInfo& CvGlobals::getActionInfo(int i)
{
	FAssertMsg(i < getNumActionInfos(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return *(m_paActionInfo[i]);
}

std::vector<CvMissionInfo*>& CvGlobals::getMissionInfo()	// For Moose - XML Load Util, CvInfos
{
	return m_paMissionInfo;
}

CvMissionInfo& CvGlobals::getMissionInfo(MissionTypes eMissionNum)
{
	FAssert(eMissionNum > -1);
	FAssert(eMissionNum < NUM_MISSION_TYPES);
	return *(m_paMissionInfo[eMissionNum]);
}

std::vector<CvControlInfo*>& CvGlobals::getControlInfo()	// For Moose - XML Load Util, CvInfos
{
	return m_paControlInfo;
}

CvControlInfo& CvGlobals::getControlInfo(ControlTypes eControlNum)
{
	FAssert(eControlNum > -1);
	FAssert(eControlNum < NUM_CONTROL_TYPES);
	FAssert(m_paControlInfo.size() > 0);
	return *(m_paControlInfo[eControlNum]);
}

std::vector<CvCommandInfo*>& CvGlobals::getCommandInfo()	// For Moose - XML Load Util, CvInfos
{
	return m_paCommandInfo;
}

CvCommandInfo& CvGlobals::getCommandInfo(CommandTypes eCommandNum)
{
	FAssert(eCommandNum > -1);
	FAssert(eCommandNum < NUM_COMMAND_TYPES);
	return *(m_paCommandInfo[eCommandNum]);
}

int CvGlobals::getNumAutomateInfos()
{
	return (int)m_paAutomateInfo.size();
}

std::vector<CvAutomateInfo*>& CvGlobals::getAutomateInfo()	// For Moose - XML Load Util, CvInfos
{
	return m_paAutomateInfo;
}

CvAutomateInfo& CvGlobals::getAutomateInfo(int iAutomateNum)
{
	FAssertMsg(iAutomateNum < getNumAutomateInfos(), "Index out of bounds");
	FAssertMsg(iAutomateNum > -1, "Index out of bounds");
	return *(m_paAutomateInfo[iAutomateNum]);
}

int CvGlobals::getNumPromotionInfos()
{
	return (int)m_paPromotionInfo.size();
}

std::vector<CvPromotionInfo*>& CvGlobals::getPromotionInfo()	// For Moose - XML Load Util, CvInfos
{
	return m_paPromotionInfo;
}

CvPromotionInfo& CvGlobals::getPromotionInfo(PromotionTypes ePromotionNum)
{
	FAssert(ePromotionNum > -1);
	FAssert(ePromotionNum < GC.getNumPromotionInfos());
	return *(m_paPromotionInfo[ePromotionNum]);
}

//FfH: Added by Kael 07/23/2007
int CvGlobals::getNumAlignmentInfos()
{
	return (int)m_paAlignmentInfo.size();
}

std::vector<CvInfoBase*>& CvGlobals::getAlignmentInfo()
{
	return m_paAlignmentInfo;
}

CvInfoBase& CvGlobals::getAlignmentInfo(AlignmentTypes eAlignmentNum)
{
	FAssert(eAlignmentNum > -1);
	FAssert(eAlignmentNum < NUM_ALIGNMENT_TYPES);
	return *(m_paAlignmentInfo[eAlignmentNum]);
}

int CvGlobals::getNumDamageTypeInfos()
{
	return (int)m_paDamageTypeInfo.size();
}

std::vector<CvInfoBase*>& CvGlobals::getDamageTypeInfo()
{
	return m_paDamageTypeInfo;
}

CvInfoBase& CvGlobals::getDamageTypeInfo(DamageTypes eDamageNum)
{
	FAssert(eDamageNum > -1);
	FAssert(eDamageNum < GC.getNumDamageTypeInfos());
	return *(m_paDamageTypeInfo[eDamageNum]);
}

int CvGlobals::getNumSpellInfos()
{
	return (int)m_paSpellInfo.size();
}

std::vector<CvSpellInfo*>& CvGlobals::getSpellInfo()
{
	return m_paSpellInfo;
}

CvSpellInfo& CvGlobals::getSpellInfo(SpellTypes eSpellNum)
{
	FAssert(eSpellNum > -1);
	FAssert(eSpellNum < GC.getNumSpellInfos());
	return *(m_paSpellInfo[eSpellNum]);
}
//FfH: End Add

/*************************************************************************************************/
/**	BETTER AI (Better Promotions) Sephi                                      					**/
/**																								**/
/**						                                            							**/
/*************************************************************************************************/
int CvGlobals::getNumAIPromotionSpecializationInfos()
{
    return (int)m_paAIPromotionSpecializationInfo.size();
}

std::vector<CvAIPromotionSpecializationInfo*>& CvGlobals::getAIPromotionSpecializationInfo()
{
    return m_paAIPromotionSpecializationInfo;
}

CvAIPromotionSpecializationInfo& CvGlobals::getAIPromotionSpecializationInfo(AIPromotionSpecializationTypes eAIPromotionSpecializationNum)
{
	FAssert(eAIPromotionSpecializationNum > -1);
	FAssert(eAIPromotionSpecializationNum < GC.getNumAIPromotionSpecializationInfos());
	return *(m_paAIPromotionSpecializationInfo[eAIPromotionSpecializationNum]);
}

/**	ADDON (Adventure) Sephi                                                      					**/
int CvGlobals::getNumAdventureInfos()
{
    return (int)m_paAdventureInfo.size();
}

std::vector<CvAdventureInfo*>& CvGlobals::getAdventureInfo()
{
    return m_paAdventureInfo;
}

CvAdventureInfo& CvGlobals::getAdventureInfo(AdventureTypes eAdventureNum)
{
	FAssert(eAdventureNum > -1);
	FAssert(eAdventureNum < GC.getNumAdventureInfos());
	return *(m_paAdventureInfo[eAdventureNum]);
}

int CvGlobals::getNumAdventureStepInfos()
{
    return (int)m_paAdventureStepInfo.size();
}

std::vector<CvAdventureStepInfo*>& CvGlobals::getAdventureStepInfo()
{
    return m_paAdventureStepInfo;
}

CvAdventureStepInfo& CvGlobals::getAdventureStepInfo(AdventureStepTypes eAdventureStepNum)
{
	FAssert(eAdventureStepNum > -1);
	FAssert(eAdventureStepNum < GC.getNumAdventureStepInfos());
	return *(m_paAdventureStepInfo[eAdventureStepNum]);
}

/**	ADDON (Mana Schools) Sephi                                                      					**/

int CvGlobals::getNumManaschoolInfos()
{
    return (int)m_paManaschoolInfo.size();
}

std::vector<CvManaschoolInfo*>& CvGlobals::getManaschoolInfo()
{
    return m_paManaschoolInfo;
}

CvManaschoolInfo& CvGlobals::getManaschoolInfo(ManaschoolTypes eManaschoolNum)
{
	FAssert(eManaschoolNum > -1);
	FAssert(eManaschoolNum < GC.getNumManaschoolInfos());
	return *(m_paManaschoolInfo[eManaschoolNum]);
}

/**	ADDON (Combatauras) Sephi																	**/

int CvGlobals::getNumCombatAuraInfos()
{
    return (int)m_paCombatAuraInfo.size();
}

std::vector<CvCombatAuraInfo*>& CvGlobals::getCombatAuraInfo()
{
    return m_paCombatAuraInfo;
}

CvCombatAuraInfo& CvGlobals::getCombatAuraInfo(CombatAuraTypes eCombatAuraNum)
{
	FAssert(eCombatAuraNum > -1);
	FAssert(eCombatAuraNum < GC.getNumCombatAuraInfos());
	return *(m_paCombatAuraInfo[eCombatAuraNum]);
}

int CvGlobals::getNumCombatAuraClassInfos()
{
    return (int)m_paCombatAuraClassInfo.size();
}

std::vector<CvCombatAuraClassInfo*>& CvGlobals::getCombatAuraClassInfo()
{
    return m_paCombatAuraClassInfo;
}

CvCombatAuraClassInfo& CvGlobals::getCombatAuraClassInfo(CombatAuraClassTypes eCombatAuraClassNum)
{
	FAssert(eCombatAuraClassNum > -1);
	FAssert(eCombatAuraClassNum < GC.getNumCombatAuraClassInfos());
	return *(m_paCombatAuraClassInfo[eCombatAuraClassNum]);
}

int CvGlobals::getNumEquipmentCategoryInfos()
{
    return (int)m_paEquipmentCategoryInfo.size();
}

std::vector<CvEquipmentCategoryInfo*>& CvGlobals::getEquipmentCategoryInfo()
{
    return m_paEquipmentCategoryInfo;
}

CvEquipmentCategoryInfo& CvGlobals::getEquipmentCategoryInfo(EquipmentCategoryTypes eEquipmentCategoryNum)
{
	FAssert(eEquipmentCategoryNum > -1);
	FAssert(eEquipmentCategoryNum < GC.getNumEquipmentCategoryInfos());
	return *(m_paEquipmentCategoryInfo[eEquipmentCategoryNum]);
}

int CvGlobals::getNumDurabilityInfos()
{
    return (int)m_paDurabilityInfo.size();
}

std::vector<CvDurabilityInfo*>& CvGlobals::getDurabilityInfo()
{
    return m_paDurabilityInfo;
}

CvDurabilityInfo& CvGlobals::getDurabilityInfo(DurabilityTypes eDurabilityNum)
{
	FAssert(eDurabilityNum > -1);
	FAssert(eDurabilityNum < GC.getNumDurabilityInfos());
	return *(m_paDurabilityInfo[eDurabilityNum]);
}

int CvGlobals::getNumDungeonInfos()
{
    return (int)m_paDungeonInfo.size();
}

std::vector<CvDungeonInfo*>& CvGlobals::getDungeonInfo()
{
    return m_paDungeonInfo;
}

CvDungeonInfo& CvGlobals::getDungeonInfo(DungeonTypes eDungeonNum)
{
	FAssert(eDungeonNum > -1);
	FAssert(eDungeonNum < GC.getNumDungeonInfos());
	return *(m_paDungeonInfo[eDungeonNum]);
}

int CvGlobals::getNumDungeonEventInfos()
{
    return (int)m_paDungeonEventInfo.size();
}

std::vector<CvDungeonEventInfo*>& CvGlobals::getDungeonEventInfo()
{
    return m_paDungeonEventInfo;
}

CvDungeonEventInfo& CvGlobals::getDungeonEventInfo(DungeonEventTypes eDungeonNum)
{
	FAssert(eDungeonNum > -1);
	FAssert(eDungeonNum < GC.getNumDungeonEventInfos());
	return *(m_paDungeonEventInfo[eDungeonNum]);
}


int CvGlobals::getNumLairResultInfos()
{
    return (int)m_paLairResultInfo.size();
}

std::vector<CvLairResultInfo*>& CvGlobals::getLairResultInfo()
{
    return m_paLairResultInfo;
}

CvLairResultInfo& CvGlobals::getLairResultInfo(LairResultTypes eLairResultNum)
{
	FAssert(eLairResultNum > -1);
	FAssert(eLairResultNum < GC.getNumLairResultInfos());
	return *(m_paLairResultInfo[eLairResultNum]);
}

int CvGlobals::getNumCityStateInfos()
{
    return (int)m_paCityStateInfo.size();
}

std::vector<CvCityStateInfo*>& CvGlobals::getCityStateInfo()
{
    return m_paCityStateInfo;
}

CvCityStateInfo& CvGlobals::getCityStateInfo(CityStateTypes eCityStateNum)
{
	FAssert(eCityStateNum > -1);
	FAssert(eCityStateNum < GC.getNumCityStateInfos());
	return *(m_paCityStateInfo[eCityStateNum]);
}

int CvGlobals::getNumPlaneInfos()
{
    return (int)m_paPlaneInfo.size();
}

std::vector<CvPlaneInfo*>& CvGlobals::getPlaneInfo()
{
    return m_paPlaneInfo;
}

CvPlaneInfo& CvGlobals::getPlaneInfo(PlaneTypes ePlaneNum)
{
	FAssert(ePlaneNum > -1);
	FAssert(ePlaneNum < GC.getNumPlaneInfos());
	return *(m_paPlaneInfo[ePlaneNum]);
}

int CvGlobals::getNumTerrainClassInfos()
{
    return (int)m_paTerrainClassInfo.size();
}

std::vector<CvTerrainClassInfo*>& CvGlobals::getTerrainClassInfo()
{
    return m_paTerrainClassInfo;
}

CvTerrainClassInfo& CvGlobals::getTerrainClassInfo(TerrainClassTypes eTerrainClassNum)
{
	FAssert(eTerrainClassNum > -1);
	FAssert(eTerrainClassNum < GC.getNumTerrainClassInfos());
	return *(m_paTerrainClassInfo[eTerrainClassNum]);
}

int CvGlobals::getNumFeatureClassInfos()
{
    return (int)m_paFeatureClassInfo.size();
}

std::vector<CvFeatureClassInfo*>& CvGlobals::getFeatureClassInfo()
{
    return m_paFeatureClassInfo;
}

CvFeatureClassInfo& CvGlobals::getFeatureClassInfo(FeatureClassTypes eFeatureClassNum)
{
	FAssert(eFeatureClassNum > -1);
	FAssert(eFeatureClassNum < GC.getNumFeatureClassInfos());
	return *(m_paFeatureClassInfo[eFeatureClassNum]);
}

int CvGlobals::getNumCitySpecializationInfos()
{
    return (int)m_paCitySpecializationInfo.size();
}

std::vector<CvCitySpecializationInfo*>& CvGlobals::getCitySpecializationInfo()
{
    return m_paCitySpecializationInfo;
}

CvCitySpecializationInfo& CvGlobals::getCitySpecializationInfo(CitySpecializationTypes eCitySpecializationNum)
{
	FAssert(eCitySpecializationNum > -1);
	FAssert(eCitySpecializationNum < GC.getNumCitySpecializationInfos());
	return *(m_paCitySpecializationInfo[eCitySpecializationNum]);
}

/*************************************************************************************************/
/**	END	                                        												**/
/*************************************************************************************************/


int CvGlobals::getNumTechInfos()
{
	return (int)m_paTechInfo.size();
}

std::vector<CvTechInfo*>& CvGlobals::getTechInfo()	// For Moose - XML Load Util, CvInfos, CvCacheObject
{
	return m_paTechInfo;
}

CvTechInfo& CvGlobals::getTechInfo(TechTypes eTechNum)
{
	FAssert(eTechNum > -1);
	FAssert(eTechNum < GC.getNumTechInfos());
	return *(m_paTechInfo[eTechNum]);
}

int CvGlobals::getNumReligionInfos()
{
	return (int)m_paReligionInfo.size();
}

std::vector<CvReligionInfo*>& CvGlobals::getReligionInfo()	// For Moose - XML Load Util, CvInfos
{
	return m_paReligionInfo;
}

CvReligionInfo& CvGlobals::getReligionInfo(ReligionTypes eReligionNum)
{
	FAssert(eReligionNum > -1);
	FAssert(eReligionNum < GC.getNumReligionInfos());
	return *(m_paReligionInfo[eReligionNum]);
}

int CvGlobals::getNumCorporationInfos()
{
	return (int)m_paCorporationInfo.size();
}

std::vector<CvCorporationInfo*>& CvGlobals::getCorporationInfo()	// For Moose - XML Load Util, CvInfos
{
	return m_paCorporationInfo;
}

CvCorporationInfo& CvGlobals::getCorporationInfo(CorporationTypes eCorporationNum)
{
	FAssert(eCorporationNum > -1);
	FAssert(eCorporationNum < GC.getNumCorporationInfos());
	return *(m_paCorporationInfo[eCorporationNum]);
}

int CvGlobals::getNumSpecialistInfos()
{
	return (int)m_paSpecialistInfo.size();
}

std::vector<CvSpecialistInfo*>& CvGlobals::getSpecialistInfo()	// For Moose - XML Load Util, CvInfos
{
	return m_paSpecialistInfo;
}

CvSpecialistInfo& CvGlobals::getSpecialistInfo(SpecialistTypes eSpecialistNum)
{
	FAssert(eSpecialistNum > -1);
	FAssert(eSpecialistNum < GC.getNumSpecialistInfos());
	return *(m_paSpecialistInfo[eSpecialistNum]);
}

int CvGlobals::getNumCivicOptionInfos()
{
	return (int)m_paCivicOptionInfo.size();
}

std::vector<CvCivicOptionInfo*>& CvGlobals::getCivicOptionInfo()	// For Moose - XML Load Util, CvInfos
{
	return m_paCivicOptionInfo;
}

CvCivicOptionInfo& CvGlobals::getCivicOptionInfo(CivicOptionTypes eCivicOptionNum)
{
	FAssert(eCivicOptionNum > -1);
	FAssert(eCivicOptionNum < GC.getNumCivicOptionInfos());
	return *(m_paCivicOptionInfo[eCivicOptionNum]);
}

int CvGlobals::getNumCivicInfos()
{
	return (int)m_paCivicInfo.size();
}

std::vector<CvCivicInfo*>& CvGlobals::getCivicInfo()	// For Moose - XML Load Util, CvInfos
{
	return m_paCivicInfo;
}

CvCivicInfo& CvGlobals::getCivicInfo(CivicTypes eCivicNum)
{
	FAssert(eCivicNum > -1);
	FAssert(eCivicNum < GC.getNumCivicInfos());
	return *(m_paCivicInfo[eCivicNum]);
}

int CvGlobals::getNumDiplomacyInfos()
{
	return (int)m_paDiplomacyInfo.size();
}

std::vector<CvDiplomacyInfo*>& CvGlobals::getDiplomacyInfo()	// For Moose - XML Load Util, CvInfos
{
	return m_paDiplomacyInfo;
}

CvDiplomacyInfo& CvGlobals::getDiplomacyInfo(int iDiplomacyNum)
{
	FAssertMsg(iDiplomacyNum < getNumDiplomacyInfos(), "Index out of bounds");
	FAssertMsg(iDiplomacyNum > -1, "Index out of bounds");
	return *(m_paDiplomacyInfo[iDiplomacyNum]);
}

int CvGlobals::getNumEraInfos()
{
	return (int)m_aEraInfo.size();
}

std::vector<CvEraInfo*>& CvGlobals::getEraInfo()	// For Moose - XML Load Util, CvInfos
{
	return m_aEraInfo;
}

CvEraInfo& CvGlobals::getEraInfo(EraTypes eEraNum)
{
	FAssert(eEraNum > -1);
	FAssert(eEraNum < GC.getNumEraInfos());
	return *(m_aEraInfo[eEraNum]);
}

int CvGlobals::getNumHurryInfos()
{
	return (int)m_paHurryInfo.size();
}

std::vector<CvHurryInfo*>& CvGlobals::getHurryInfo()	// For Moose - XML Load Util, CvInfos
{
	return m_paHurryInfo;
}

CvHurryInfo& CvGlobals::getHurryInfo(HurryTypes eHurryNum)
{
	FAssert(eHurryNum > -1);
	FAssert(eHurryNum < GC.getNumHurryInfos());
	return *(m_paHurryInfo[eHurryNum]);
}

int CvGlobals::getNumEmphasizeInfos()
{
	return (int)m_paEmphasizeInfo.size();
}

std::vector<CvEmphasizeInfo*>& CvGlobals::getEmphasizeInfo()	// For Moose - XML Load Util
{
	return m_paEmphasizeInfo;
}

CvEmphasizeInfo& CvGlobals::getEmphasizeInfo(EmphasizeTypes eEmphasizeNum)
{
	FAssert(eEmphasizeNum > -1);
	FAssert(eEmphasizeNum < GC.getNumEmphasizeInfos());
	return *(m_paEmphasizeInfo[eEmphasizeNum]);
}

int CvGlobals::getNumUpkeepInfos()
{
	return (int)m_paUpkeepInfo.size();
}

std::vector<CvUpkeepInfo*>& CvGlobals::getUpkeepInfo()	// For Moose - XML Load Util, CvInfos
{
	return m_paUpkeepInfo;
}

CvUpkeepInfo& CvGlobals::getUpkeepInfo(UpkeepTypes eUpkeepNum)
{
	FAssert(eUpkeepNum > -1);
	FAssert(eUpkeepNum < GC.getNumUpkeepInfos());
	return *(m_paUpkeepInfo[eUpkeepNum]);
}

int CvGlobals::getNumCultureLevelInfos()
{
	return (int)m_paCultureLevelInfo.size();
}

std::vector<CvCultureLevelInfo*>& CvGlobals::getCultureLevelInfo()	// For Moose - XML Load Util, CvInfos
{
	return m_paCultureLevelInfo;
}

CvCultureLevelInfo& CvGlobals::getCultureLevelInfo(CultureLevelTypes eCultureLevelNum)
{
	FAssert(eCultureLevelNum > -1);
	FAssert(eCultureLevelNum < GC.getNumCultureLevelInfos());
	return *(m_paCultureLevelInfo[eCultureLevelNum]);
}

int CvGlobals::getNumVictoryInfos()
{
	return (int)m_paVictoryInfo.size();
}

std::vector<CvVictoryInfo*>& CvGlobals::getVictoryInfo()	// For Moose - XML Load Util, CvInfos
{
	return m_paVictoryInfo;
}

CvVictoryInfo& CvGlobals::getVictoryInfo(VictoryTypes eVictoryNum)
{
	FAssert(eVictoryNum > -1);
	FAssert(eVictoryNum < GC.getNumVictoryInfos());
	return *(m_paVictoryInfo[eVictoryNum]);
}

int CvGlobals::getNumQuestInfos()
{
	return (int)m_paQuestInfo.size();
}

std::vector<CvQuestInfo*>& CvGlobals::getQuestInfo()
{
	return m_paQuestInfo;
}

CvQuestInfo& CvGlobals::getQuestInfo(int iIndex)
{
	FAssert(iIndex > -1);
	FAssert(iIndex < GC.getNumQuestInfos());
	return *(m_paQuestInfo[iIndex]);
}

int CvGlobals::getNumTutorialInfos()
{
	return (int)m_paTutorialInfo.size();
}

std::vector<CvTutorialInfo*>& CvGlobals::getTutorialInfo()
{
	return m_paTutorialInfo;
}

CvTutorialInfo& CvGlobals::getTutorialInfo(int iIndex)
{
	FAssert(iIndex > -1);
	FAssert(iIndex < GC.getNumTutorialInfos());
	return *(m_paTutorialInfo[iIndex]);
}

int CvGlobals::getNumEventTriggerInfos()
{
	return (int)m_paEventTriggerInfo.size();
}

std::vector<CvEventTriggerInfo*>& CvGlobals::getEventTriggerInfo()
{
	return m_paEventTriggerInfo;
}

CvEventTriggerInfo& CvGlobals::getEventTriggerInfo(EventTriggerTypes eEventTrigger)
{
	FAssert(eEventTrigger > -1);
	FAssert(eEventTrigger < GC.getNumEventTriggerInfos());
	return *(m_paEventTriggerInfo[eEventTrigger]);
}

int CvGlobals::getNumEventInfos()
{
	return (int)m_paEventInfo.size();
}

std::vector<CvEventInfo*>& CvGlobals::getEventInfo()
{
	return m_paEventInfo;
}

CvEventInfo& CvGlobals::getEventInfo(EventTypes eEvent)
{
	FAssert(eEvent > -1);
	FAssert(eEvent < GC.getNumEventInfos());
	return *(m_paEventInfo[eEvent]);
}

int CvGlobals::getNumEspionageMissionInfos()
{
	return (int)m_paEspionageMissionInfo.size();
}

std::vector<CvEspionageMissionInfo*>& CvGlobals::getEspionageMissionInfo()
{
	return m_paEspionageMissionInfo;
}

CvEspionageMissionInfo& CvGlobals::getEspionageMissionInfo(EspionageMissionTypes eEspionageMissionNum)
{
	FAssert(eEspionageMissionNum > -1);
	FAssert(eEspionageMissionNum < GC.getNumEspionageMissionInfos());
	return *(m_paEspionageMissionInfo[eEspionageMissionNum]);
}

int& CvGlobals::getNumEntityEventTypes()
{
	return m_iNumEntityEventTypes;
}

CvString*& CvGlobals::getEntityEventTypes()
{
	return m_paszEntityEventTypes;
}

CvString& CvGlobals::getEntityEventTypes(EntityEventTypes e)
{
	FAssert(e > -1);
	FAssert(e < GC.getNumEntityEventTypes());
	return m_paszEntityEventTypes[e];
}

int& CvGlobals::getNumAnimationOperatorTypes()
{
	return m_iNumAnimationOperatorTypes;
}

CvString*& CvGlobals::getAnimationOperatorTypes()
{
	return m_paszAnimationOperatorTypes;
}

CvString& CvGlobals::getAnimationOperatorTypes(AnimationOperatorTypes e)
{
	FAssert(e > -1);
	FAssert(e < GC.getNumAnimationOperatorTypes());
	return m_paszAnimationOperatorTypes[e];
}

CvString*& CvGlobals::getFunctionTypes()
{
	return m_paszFunctionTypes;
}

CvString& CvGlobals::getFunctionTypes(FunctionTypes e)
{
	FAssert(e > -1);
	FAssert(e < NUM_FUNC_TYPES);
	return m_paszFunctionTypes[e];
}

int& CvGlobals::getNumFlavorTypes()
{
	return m_iNumFlavorTypes;
}

CvString*& CvGlobals::getFlavorTypes()
{
	return m_paszFlavorTypes;
}

CvString& CvGlobals::getFlavorTypes(FlavorTypes e)
{
	FAssert(e > -1);
	FAssert(e < GC.getNumFlavorTypes());
	return m_paszFlavorTypes[e];
}

int& CvGlobals::getNumArtStyleTypes()
{
	return m_iNumArtStyleTypes;
}

CvString*& CvGlobals::getArtStyleTypes()
{
	return m_paszArtStyleTypes;
}

CvString& CvGlobals::getArtStyleTypes(ArtStyleTypes e)
{
	FAssert(e > -1);
	FAssert(e < GC.getNumArtStyleTypes());
	return m_paszArtStyleTypes[e];
}

int CvGlobals::getNumUnitArtStyleTypeInfos()
{
    return (int)m_paUnitArtStyleTypeInfo.size();
}

std::vector<CvUnitArtStyleTypeInfo*>& CvGlobals::getUnitArtStyleTypeInfo()
{
	return m_paUnitArtStyleTypeInfo;
}

CvUnitArtStyleTypeInfo& CvGlobals::getUnitArtStyleTypeInfo(UnitArtStyleTypes eUnitArtStyleTypeNum)
{
	FAssert(eUnitArtStyleTypeNum > -1);
	FAssert(eUnitArtStyleTypeNum < GC.getNumUnitArtStyleTypeInfos());
	return *(m_paUnitArtStyleTypeInfo[eUnitArtStyleTypeNum]);
}

int& CvGlobals::getNumCitySizeTypes()
{
	return m_iNumCitySizeTypes;
}

CvString*& CvGlobals::getCitySizeTypes()
{
	return m_paszCitySizeTypes;
}

CvString& CvGlobals::getCitySizeTypes(int i)
{
	FAssertMsg(i < getNumCitySizeTypes(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_paszCitySizeTypes[i];
}

CvString*& CvGlobals::getContactTypes()
{
	return m_paszContactTypes;
}

CvString& CvGlobals::getContactTypes(ContactTypes e)
{
	FAssert(e > -1);
	FAssert(e < NUM_CONTACT_TYPES);
	return m_paszContactTypes[e];
}

CvString*& CvGlobals::getDiplomacyPowerTypes()
{
	return m_paszDiplomacyPowerTypes;
}

CvString& CvGlobals::getDiplomacyPowerTypes(DiplomacyPowerTypes e)
{
	FAssert(e > -1);
	FAssert(e < NUM_DIPLOMACYPOWER_TYPES);
	return m_paszDiplomacyPowerTypes[e];
}

CvString*& CvGlobals::getAutomateTypes()
{
	return m_paszAutomateTypes;
}

CvString& CvGlobals::getAutomateTypes(AutomateTypes e)
{
	FAssert(e > -1);
	FAssert(e < NUM_AUTOMATE_TYPES);
	return m_paszAutomateTypes[e];
}

CvString*& CvGlobals::getDirectionTypes()
{
	return m_paszDirectionTypes;
}

CvString& CvGlobals::getDirectionTypes(AutomateTypes e)
{
	FAssert(e > -1);
	FAssert(e < NUM_DIRECTION_TYPES);
	return m_paszDirectionTypes[e];
}

int& CvGlobals::getNumFootstepAudioTypes()
{
	return m_iNumFootstepAudioTypes;
}

CvString*& CvGlobals::getFootstepAudioTypes()
{
	return m_paszFootstepAudioTypes;
}

CvString& CvGlobals::getFootstepAudioTypes(int i)
{
	FAssertMsg(i < getNumFootstepAudioTypes(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_paszFootstepAudioTypes[i];
}

int CvGlobals::getFootstepAudioTypeByTag(CvString strTag)
{
	int iIndex = -1;

	if ( strTag.GetLength() <= 0 )
	{
		return iIndex;
	}

	for ( int i = 0; i < m_iNumFootstepAudioTypes; i++ )
	{
		if ( strTag.CompareNoCase(m_paszFootstepAudioTypes[i]) == 0 )
		{
			iIndex = i;
			break;
		}
	}

	return iIndex;
}

CvString*& CvGlobals::getFootstepAudioTags()
{
	return m_paszFootstepAudioTags;
}

CvString& CvGlobals::getFootstepAudioTags(int i)
{
//	FAssertMsg(i < getNumFootstepAudioTags(), "Index out of bounds")
	FAssertMsg(i > -1, "Index out of bounds");
	return m_paszFootstepAudioTags[i];
}

void CvGlobals::setCurrentXMLFile(const TCHAR* szFileName)
{
	m_szCurrentXMLFile = szFileName;
}

CvString& CvGlobals::getCurrentXMLFile()
{
	return m_szCurrentXMLFile;
}

FVariableSystem* CvGlobals::getDefinesVarSystem()
{
	return m_VarSystem;
}

void CvGlobals::cacheGlobals()
{
	cacheGlobals(false);
}

void CvGlobals::cacheGlobals(bool bXMLLoaded)
{
	m_iMOVE_DENOMINATOR = getDefineINT("MOVE_DENOMINATOR");
	m_iNUM_UNIT_PREREQ_OR_BONUSES = getDefineINT("NUM_UNIT_PREREQ_OR_BONUSES");
	m_iNUM_BUILDING_PREREQ_OR_BONUSES = getDefineINT("NUM_BUILDING_PREREQ_OR_BONUSES");
	m_iFOOD_CONSUMPTION_PER_POPULATION = getDefineINT("FOOD_CONSUMPTION_PER_POPULATION");
	m_iMAX_HIT_POINTS = getDefineINT("MAX_HIT_POINTS");
	m_iPATH_DAMAGE_WEIGHT = getDefineINT("PATH_DAMAGE_WEIGHT");
	m_iHILLS_EXTRA_DEFENSE = getDefineINT("HILLS_EXTRA_DEFENSE");
	m_iRIVER_ATTACK_MODIFIER = getDefineINT("RIVER_ATTACK_MODIFIER");
	m_iAMPHIB_ATTACK_MODIFIER = getDefineINT("AMPHIB_ATTACK_MODIFIER");
	m_iHILLS_EXTRA_MOVEMENT = getDefineINT("HILLS_EXTRA_MOVEMENT");
	m_iMAX_PLOT_LIST_ROWS = getDefineINT("MAX_PLOT_LIST_ROWS");
	m_iUNIT_MULTISELECT_MAX = getDefineINT("UNIT_MULTISELECT_MAX");
	m_iPERCENT_ANGER_DIVISOR = getDefineINT("PERCENT_ANGER_DIVISOR");
	m_iEVENT_MESSAGE_TIME = getDefineINT("EVENT_MESSAGE_TIME");
	m_iROUTE_FEATURE_GROWTH_MODIFIER = getDefineINT("ROUTE_FEATURE_GROWTH_MODIFIER");
	m_iFEATURE_GROWTH_MODIFIER = getDefineINT("FEATURE_GROWTH_MODIFIER");
	m_iMIN_CITY_RANGE = getDefineINT("MIN_CITY_RANGE");
	m_iCITY_MAX_NUM_BUILDINGS = getDefineINT("CITY_MAX_NUM_BUILDINGS");
	m_iNUM_UNIT_AND_TECH_PREREQS = getDefineINT("NUM_UNIT_AND_TECH_PREREQS");
	m_iNUM_AND_TECH_PREREQS = getDefineINT("NUM_AND_TECH_PREREQS");
	m_iNUM_OR_TECH_PREREQS = getDefineINT("NUM_OR_TECH_PREREQS");
	m_iLAKE_MAX_AREA_SIZE = getDefineINT("LAKE_MAX_AREA_SIZE");
	m_iNUM_ROUTE_PREREQ_OR_BONUSES = getDefineINT("NUM_ROUTE_PREREQ_OR_BONUSES");
	m_iNUM_BUILDING_AND_TECH_PREREQS = getDefineINT("NUM_BUILDING_AND_TECH_PREREQS");
	m_iMIN_WATER_SIZE_FOR_OCEAN = getDefineINT("MIN_WATER_SIZE_FOR_OCEAN");
	m_iFORTIFY_MODIFIER_PER_TURN = getDefineINT("FORTIFY_MODIFIER_PER_TURN");
	m_iMAX_CITY_DEFENSE_DAMAGE = getDefineINT("MAX_CITY_DEFENSE_DAMAGE");
	m_iNUM_CORPORATION_PREREQ_BONUSES = getDefineINT("NUM_CORPORATION_PREREQ_BONUSES");
	m_iPEAK_SEE_THROUGH_CHANGE = getDefineINT("PEAK_SEE_THROUGH_CHANGE");
	m_iHILLS_SEE_THROUGH_CHANGE = getDefineINT("HILLS_SEE_THROUGH_CHANGE");
	m_iSEAWATER_SEE_FROM_CHANGE = getDefineINT("SEAWATER_SEE_FROM_CHANGE");
	m_iPEAK_SEE_FROM_CHANGE = getDefineINT("PEAK_SEE_FROM_CHANGE");
	m_iHILLS_SEE_FROM_CHANGE = getDefineINT("HILLS_SEE_FROM_CHANGE");
	m_iUSE_SPIES_NO_ENTER_BORDERS = getDefineINT("USE_SPIES_NO_ENTER_BORDERS");

	m_fCAMERA_MIN_YAW = getDefineFLOAT("CAMERA_MIN_YAW");
	m_fCAMERA_MAX_YAW = getDefineFLOAT("CAMERA_MAX_YAW");
	m_fCAMERA_FAR_CLIP_Z_HEIGHT = getDefineFLOAT("CAMERA_FAR_CLIP_Z_HEIGHT");
	m_fCAMERA_MAX_TRAVEL_DISTANCE = getDefineFLOAT("CAMERA_MAX_TRAVEL_DISTANCE");
	m_fCAMERA_START_DISTANCE = getDefineFLOAT("CAMERA_START_DISTANCE");
	m_fAIR_BOMB_HEIGHT = getDefineFLOAT("AIR_BOMB_HEIGHT");
	m_fPLOT_SIZE = getDefineFLOAT("PLOT_SIZE");
	m_fCAMERA_SPECIAL_PITCH = getDefineFLOAT("CAMERA_SPECIAL_PITCH");
	m_fCAMERA_MAX_TURN_OFFSET = getDefineFLOAT("CAMERA_MAX_TURN_OFFSET");
	m_fCAMERA_MIN_DISTANCE = getDefineFLOAT("CAMERA_MIN_DISTANCE");
	m_fCAMERA_UPPER_PITCH = getDefineFLOAT("CAMERA_UPPER_PITCH");
	m_fCAMERA_LOWER_PITCH = getDefineFLOAT("CAMERA_LOWER_PITCH");
	m_fFIELD_OF_VIEW = getDefineFLOAT("FIELD_OF_VIEW");
	m_fSHADOW_SCALE = getDefineFLOAT("SHADOW_SCALE");
	m_fUNIT_MULTISELECT_DISTANCE = getDefineFLOAT("UNIT_MULTISELECT_DISTANCE");

	m_iUSE_CANNOT_FOUND_CITY_CALLBACK = getDefineINT("USE_CANNOT_FOUND_CITY_CALLBACK");
	m_iUSE_CAN_FOUND_CITIES_ON_WATER_CALLBACK = getDefineINT("USE_CAN_FOUND_CITIES_ON_WATER_CALLBACK");
	m_iUSE_IS_PLAYER_RESEARCH_CALLBACK = getDefineINT("USE_IS_PLAYER_RESEARCH_CALLBACK");
	m_iUSE_CAN_RESEARCH_CALLBACK = getDefineINT("USE_CAN_RESEARCH_CALLBACK");
	m_iUSE_CANNOT_DO_CIVIC_CALLBACK = getDefineINT("USE_CANNOT_DO_CIVIC_CALLBACK");
	m_iUSE_CAN_DO_CIVIC_CALLBACK = getDefineINT("USE_CAN_DO_CIVIC_CALLBACK");
	m_iUSE_CANNOT_CONSTRUCT_CALLBACK = getDefineINT("USE_CANNOT_CONSTRUCT_CALLBACK");
	m_iUSE_CAN_CONSTRUCT_CALLBACK = getDefineINT("USE_CAN_CONSTRUCT_CALLBACK");
	m_iUSE_CAN_DECLARE_WAR_CALLBACK = getDefineINT("USE_CAN_DECLARE_WAR_CALLBACK");
	m_iUSE_CANNOT_RESEARCH_CALLBACK = getDefineINT("USE_CANNOT_RESEARCH_CALLBACK");
	m_iUSE_GET_UNIT_COST_MOD_CALLBACK = getDefineINT("USE_GET_UNIT_COST_MOD_CALLBACK");
	m_iUSE_GET_BUILDING_COST_MOD_CALLBACK = getDefineINT("USE_GET_BUILDING_COST_MOD_CALLBACK");
	m_iUSE_GET_CITY_FOUND_VALUE_CALLBACK = getDefineINT("USE_GET_CITY_FOUND_VALUE_CALLBACK");
	m_iUSE_CANNOT_HANDLE_ACTION_CALLBACK = getDefineINT("USE_CANNOT_HANDLE_ACTION_CALLBACK");
	m_iUSE_CAN_BUILD_CALLBACK = getDefineINT("USE_CAN_BUILD_CALLBACK");
	m_iUSE_CANNOT_TRAIN_CALLBACK = getDefineINT("USE_CANNOT_TRAIN_CALLBACK");
	m_iUSE_CAN_TRAIN_CALLBACK = getDefineINT("USE_CAN_TRAIN_CALLBACK");
	m_iUSE_UNIT_CANNOT_MOVE_INTO_CALLBACK = getDefineINT("USE_UNIT_CANNOT_MOVE_INTO_CALLBACK");
	m_iUSE_USE_CANNOT_SPREAD_RELIGION_CALLBACK = getDefineINT("USE_USE_CANNOT_SPREAD_RELIGION_CALLBACK");
	m_iUSE_FINISH_TEXT_CALLBACK = getDefineINT("USE_FINISH_TEXT_CALLBACK");
	m_iUSE_ON_UNIT_SET_XY_CALLBACK = getDefineINT("USE_ON_UNIT_SET_XY_CALLBACK");
	m_iUSE_ON_UNIT_SELECTED_CALLBACK = getDefineINT("USE_ON_UNIT_SELECTED_CALLBACK");
	m_iUSE_ON_UPDATE_CALLBACK = getDefineINT("USE_ON_UPDATE_CALLBACK");
	m_iUSE_ON_UNIT_CREATED_CALLBACK = getDefineINT("USE_ON_UNIT_CREATED_CALLBACK");
	m_iUSE_ON_UNIT_LOST_CALLBACK = getDefineINT("USE_ON_UNIT_LOST_CALLBACK");

//FfH: Added by Kael 08/02/2008
	m_iUSE_ON_UNIT_MOVE_CALLBACK = getDefineINT("USE_ON_UNIT_MOVE_CALLBACK");
	m_iUSE_PLOT_REVEALED_CALLBACK = getDefineINT("USE_PLOT_REVEALED_CALLBACK");
	m_iUSE_COMBAT_RESULT_CALLBACK = getDefineINT("USE_COMBAT_RESULT_CALLBACK");
//FfH: End Add

	m_iGraphicalDetailPageInRange = 256;//getDefineINT("GRAPHICAL_DETAIL_PAGE_IN_RANGE", 256);

	//new Cache Values - Sephi
	m_iFREE_FAVORED_IMPROVEMENTS_PER_CITY = GC.getDefineINT("FREE_FAVORED_IMPROVEMENTS_PER_CITY");
	if(bXMLLoaded) {
		m_iIMPROVEMENT_COTTAGE = GC.getDefineINT("IMPROVEMENT_COTTAGE");
		m_iIMPROVEMENT_FARM = GC.getDefineINT("IMPROVEMENT_FARM");
		m_iIMPROVEMENT_GROUNDWATER_WELL = GC.getDefineINT("IMPROVEMENT_GROUNDWATER_WELL");
		m_iIMPROVEMENT_PASTURE = GC.getDefineINT("IMPROVEMENT_PASTURE");
		m_iIMPROVEMENT_QUARRY = GC.getDefineINT("IMPROVEMENT_QUARRY");
		m_iIMPROVEMENT_WATERMILL = GC.getDefineINT("IMPROVEMENT_WATERMILL");
		m_iIMPROVEMENT_WINDMILL = GC.getDefineINT("IMPROVEMENT_WINDMILL");
		m_iIMPROVEMENT_WORKSHOP = GC.getDefineINT("IMPROVEMENT_WORKSHOP");
		m_iFEATURE_HAUNTED_LANDS = GC.getDefineINT("FEATURE_HAUNTED_LANDS");
		m_iBUILDINGCLASS_SAGE_DISTRICT = GC.getDefineINT("BUILDINGCLASS_SAGE_DISTRICT");
		m_iBUILDINGCLASS_NOBLE_DISTRICT = GC.getDefineINT("BUILDINGCLASS_NOBLE_DISTRICT");
		m_iBUILDINGCLASS_MERCHANT_DISTRICT = GC.getDefineINT("BUILDINGCLASS_MERCHANT_DISTRICT");
		m_iBUILDINGCLASS_WARRIOR_DISTRICT = GC.getDefineINT("BUILDINGCLASS_WARRIOR_DISTRICT");
		m_iBUILDINGCLASS_TEMPLE_DISTRICT = GC.getDefineINT("BUILDINGCLASS_TEMPLE_DISTRICT");
		m_iBUILDINGCLASS_BARD_DISTRICT = GC.getDefineINT("BUILDINGCLASS_BARD_DISTRICT");
		m_iBUILDINGCLASS_LUXURY_DISTRICT = GC.getDefineINT("BUILDINGCLASS_LUXURY_DISTRICT");
		m_iBUILDINGCLASS_FOREIGN_TRADE_DISTRICT = GC.getDefineINT("BUILDINGCLASS_FOREIGN_TRADE_DISTRICT");
		m_iBUILDINGCLASS_RESOURCE_DISTRICT = GC.getDefineINT("BUILDINGCLASS_RESOURCE_DISTRICT");
		m_iBUILDINGCLASS_HERBALIST = GC.getDefineINT("BUILDINGCLASS_HERBALIST");
		m_iBUILDINGCLASS_SCHOLA_ARCANA = GC.getDefineINT("BUILDINGCLASS_SCHOLA_ARCANA");
		m_iBUILDINGCLASS_SALON = GC.getDefineINT("BUILDINGCLASS_SALON");
		m_iBUILDINGCLASS_PALACE = GC.getDefineINT("BUILDINGCLASS_PALACE");
		m_iUNITCLASS_SETTLER = GC.getDefineINT("UNITCLASS_SETTLER");
		m_iUNITCLASS_WORKBOAT = GC.getDefineINT("UNITCLASS_WORKBOAT");
		m_iUNITCLASS_WORKER = GC.getDefineINT("UNITCLASS_WORKER");
		m_iEQUIPMENT_PIECES_OF_BARNAXUS = GC.getDefineINT("EQUIPMENT_PIECES_OF_BARNAXUS");
		m_iUNITCOMBAT_ADEPT = GC.getDefineINT("UNITCOMBAT_ADEPT");
		m_iSPECIALIST_MERCHANT = GC.getDefineINT("SPECIALIST_MERCHANT");
		m_iSPECIALIST_SCIENTIST = GC.getDefineINT("SPECIALIST_SCIENTIST");
		m_iSPECIALIST_ARTIST = GC.getDefineINT("SPECIALIST_ARTIST");
		m_iSPECIALIST_PRIEST = GC.getDefineINT("SPECIALIST_PRIEST");
		m_iSPECIALIST_ENGINEER = GC.getDefineINT("SPECIALIST_ENGINEER");
		m_iIMPROVEMENT_MANA_SHRINE = GC.getDefineINT("IMPROVEMENT_MANA_SHRINE");
		m_iIMPROVEMENT_TRADE1 = GC.getDefineINT("IMPROVEMENT_TRADE1");
		m_iIMPROVEMENT_CAMP = GC.getDefineINT("IMPROVEMENT_CAMP");
		m_iDEFAULT_SPECIALIST = GC.getDefineINT("DEFAULT_SPECIALIST");
	}
}

/*************************************************************************************************/
/**	TrueModular								05/26/09	Written: Mr. Genie	Imported: Xienwolf	**/
/**				INT values work fine for this, but it could be worth having sometime			**/
/**	Properly links Modular modifications to previous elements, and allows partial overwriting	**/
/*************************************************************************************************/
bool CvGlobals::getDefineBOOL( const char * szName ) const
{
	bool bReturn = false;
	GC.getDefinesVarSystem()->GetValue( szName, bReturn );
	return bReturn;
}
/*************************************************************************************************/
/**	TrueModular								END													**/
/*************************************************************************************************/
int CvGlobals::getDefineINT( const char * szName ) const
{
	int iReturn = 0;
	GC.getDefinesVarSystem()->GetValue( szName, iReturn );
	return iReturn;
}

float CvGlobals::getDefineFLOAT( const char * szName ) const
{
	float fReturn = 0;
	GC.getDefinesVarSystem()->GetValue( szName, fReturn );
	return fReturn;
}

const char * CvGlobals::getDefineSTRING( const char * szName ) const
{
	const char * szReturn = NULL;
	GC.getDefinesVarSystem()->GetValue( szName, szReturn );
	return szReturn;
}

void CvGlobals::setDefineINT( const char * szName, int iValue )
{
	GC.getDefinesVarSystem()->SetValue( szName, iValue );
	cacheGlobals();
}

void CvGlobals::setDefineFLOAT( const char * szName, float fValue )
{
	GC.getDefinesVarSystem()->SetValue( szName, fValue );
	cacheGlobals();
}

void CvGlobals::setDefineSTRING( const char * szName, const char * szValue )
{
	GC.getDefinesVarSystem()->SetValue( szName, szValue );
	cacheGlobals();
}

int CvGlobals::getMOVE_DENOMINATOR()
{
	return m_iMOVE_DENOMINATOR;
}

int CvGlobals::getNUM_UNIT_PREREQ_OR_BONUSES()
{
	return m_iNUM_UNIT_PREREQ_OR_BONUSES;
}

int CvGlobals::getNUM_BUILDING_PREREQ_OR_BONUSES()
{
	return m_iNUM_BUILDING_PREREQ_OR_BONUSES;
}

int CvGlobals::getFOOD_CONSUMPTION_PER_POPULATION()
{
	return m_iFOOD_CONSUMPTION_PER_POPULATION;
}

int CvGlobals::getMAX_HIT_POINTS()
{
	return m_iMAX_HIT_POINTS;
}

int CvGlobals::getPATH_DAMAGE_WEIGHT()
{
	return m_iPATH_DAMAGE_WEIGHT;
}

int CvGlobals::getHILLS_EXTRA_DEFENSE()
{
	return m_iHILLS_EXTRA_DEFENSE;
}

int CvGlobals::getRIVER_ATTACK_MODIFIER()
{
	return m_iRIVER_ATTACK_MODIFIER;
}

int CvGlobals::getAMPHIB_ATTACK_MODIFIER()
{
	return m_iAMPHIB_ATTACK_MODIFIER;
}

int CvGlobals::getHILLS_EXTRA_MOVEMENT()
{
	return m_iHILLS_EXTRA_MOVEMENT;
}

int CvGlobals::getMAX_PLOT_LIST_ROWS()
{
	return m_iMAX_PLOT_LIST_ROWS;
}

int CvGlobals::getUNIT_MULTISELECT_MAX()
{
	return m_iUNIT_MULTISELECT_MAX;
}

int CvGlobals::getPERCENT_ANGER_DIVISOR()
{
	return m_iPERCENT_ANGER_DIVISOR;
}

int CvGlobals::getEVENT_MESSAGE_TIME()
{
	return m_iEVENT_MESSAGE_TIME;
}

int CvGlobals::getROUTE_FEATURE_GROWTH_MODIFIER()
{
	return m_iROUTE_FEATURE_GROWTH_MODIFIER;
}

int CvGlobals::getFEATURE_GROWTH_MODIFIER()
{
	return m_iFEATURE_GROWTH_MODIFIER;
}

int CvGlobals::getMIN_CITY_RANGE()
{
	return m_iMIN_CITY_RANGE;
}

int CvGlobals::getCITY_MAX_NUM_BUILDINGS()
{
	return m_iCITY_MAX_NUM_BUILDINGS;
}

int CvGlobals::getNUM_UNIT_AND_TECH_PREREQS()
{
	return m_iNUM_UNIT_AND_TECH_PREREQS;
}

int CvGlobals::getNUM_AND_TECH_PREREQS()
{
	return m_iNUM_AND_TECH_PREREQS;
}

int CvGlobals::getNUM_OR_TECH_PREREQS()
{
	return m_iNUM_OR_TECH_PREREQS;
}

int CvGlobals::getLAKE_MAX_AREA_SIZE()
{
	return m_iLAKE_MAX_AREA_SIZE;
}

int CvGlobals::getNUM_ROUTE_PREREQ_OR_BONUSES()
{
	return m_iNUM_ROUTE_PREREQ_OR_BONUSES;
}

int CvGlobals::getNUM_BUILDING_AND_TECH_PREREQS()
{
	return m_iNUM_BUILDING_AND_TECH_PREREQS;
}

int CvGlobals::getMIN_WATER_SIZE_FOR_OCEAN()
{
	return m_iMIN_WATER_SIZE_FOR_OCEAN;
}

int CvGlobals::getFORTIFY_MODIFIER_PER_TURN()
{
	return m_iFORTIFY_MODIFIER_PER_TURN;
}

int CvGlobals::getMAX_CITY_DEFENSE_DAMAGE()
{
	return m_iMAX_CITY_DEFENSE_DAMAGE;
}

int CvGlobals::getPEAK_SEE_THROUGH_CHANGE()
{
	return m_iPEAK_SEE_THROUGH_CHANGE;
}

int CvGlobals::getHILLS_SEE_THROUGH_CHANGE()
{
	return m_iHILLS_SEE_THROUGH_CHANGE;
}

int CvGlobals::getSEAWATER_SEE_FROM_CHANGE()
{
	return m_iSEAWATER_SEE_FROM_CHANGE;
}

int CvGlobals::getPEAK_SEE_FROM_CHANGE()
{
	return m_iPEAK_SEE_FROM_CHANGE;
}

int CvGlobals::getHILLS_SEE_FROM_CHANGE()
{
	return m_iHILLS_SEE_FROM_CHANGE;
}

int CvGlobals::getUSE_SPIES_NO_ENTER_BORDERS()
{
	return m_iUSE_SPIES_NO_ENTER_BORDERS;
}

int CvGlobals::getNUM_CORPORATION_PREREQ_BONUSES()
{
	return m_iNUM_CORPORATION_PREREQ_BONUSES;
}

float CvGlobals::getCAMERA_MIN_YAW()
{
	return m_fCAMERA_MIN_YAW;
}

float CvGlobals::getCAMERA_MAX_YAW()
{
	return m_fCAMERA_MAX_YAW;
}

float CvGlobals::getCAMERA_FAR_CLIP_Z_HEIGHT()
{
	return m_fCAMERA_FAR_CLIP_Z_HEIGHT;
}

float CvGlobals::getCAMERA_MAX_TRAVEL_DISTANCE()
{
	return m_fCAMERA_MAX_TRAVEL_DISTANCE;
}

float CvGlobals::getCAMERA_START_DISTANCE()
{
	return m_fCAMERA_START_DISTANCE;
}

float CvGlobals::getAIR_BOMB_HEIGHT()
{
	return m_fAIR_BOMB_HEIGHT;
}

float CvGlobals::getPLOT_SIZE()
{
	return m_fPLOT_SIZE;
}

float CvGlobals::getCAMERA_SPECIAL_PITCH()
{
	return m_fCAMERA_SPECIAL_PITCH;
}

float CvGlobals::getCAMERA_MAX_TURN_OFFSET()
{
	return m_fCAMERA_MAX_TURN_OFFSET;
}

float CvGlobals::getCAMERA_MIN_DISTANCE()
{
	return m_fCAMERA_MIN_DISTANCE;
}

float CvGlobals::getCAMERA_UPPER_PITCH()
{
	return m_fCAMERA_UPPER_PITCH;
}

float CvGlobals::getCAMERA_LOWER_PITCH()
{
	return m_fCAMERA_LOWER_PITCH;
}

float CvGlobals::getFIELD_OF_VIEW()
{
	return m_fFIELD_OF_VIEW;
}

float CvGlobals::getSHADOW_SCALE()
{
	return m_fSHADOW_SCALE;
}

float CvGlobals::getUNIT_MULTISELECT_DISTANCE()
{
	return m_fUNIT_MULTISELECT_DISTANCE;
}

int CvGlobals::getUSE_CANNOT_FOUND_CITY_CALLBACK()
{
	return m_iUSE_CANNOT_FOUND_CITY_CALLBACK;
}

int CvGlobals::getUSE_CAN_FOUND_CITIES_ON_WATER_CALLBACK()
{
	return m_iUSE_CAN_FOUND_CITIES_ON_WATER_CALLBACK;
}

int CvGlobals::getUSE_IS_PLAYER_RESEARCH_CALLBACK()
{
	return m_iUSE_IS_PLAYER_RESEARCH_CALLBACK;
}

int CvGlobals::getUSE_CAN_RESEARCH_CALLBACK()
{
	return m_iUSE_CAN_RESEARCH_CALLBACK;
}

int CvGlobals::getUSE_CANNOT_DO_CIVIC_CALLBACK()
{
	return m_iUSE_CANNOT_DO_CIVIC_CALLBACK;
}

int CvGlobals::getUSE_CAN_DO_CIVIC_CALLBACK()
{
	return m_iUSE_CAN_DO_CIVIC_CALLBACK;
}

int CvGlobals::getUSE_CANNOT_CONSTRUCT_CALLBACK()
{
	return m_iUSE_CANNOT_CONSTRUCT_CALLBACK;
}

int CvGlobals::getUSE_CAN_CONSTRUCT_CALLBACK()
{
	return m_iUSE_CAN_CONSTRUCT_CALLBACK;
}

int CvGlobals::getUSE_CAN_DECLARE_WAR_CALLBACK()
{
	return m_iUSE_CAN_DECLARE_WAR_CALLBACK;
}

int CvGlobals::getUSE_CANNOT_RESEARCH_CALLBACK()
{
	return m_iUSE_CANNOT_RESEARCH_CALLBACK;
}

int CvGlobals::getUSE_GET_UNIT_COST_MOD_CALLBACK()
{
	return m_iUSE_GET_UNIT_COST_MOD_CALLBACK;
}

int CvGlobals::getUSE_GET_BUILDING_COST_MOD_CALLBACK()
{
	return m_iUSE_GET_BUILDING_COST_MOD_CALLBACK;
}

int CvGlobals::getUSE_GET_CITY_FOUND_VALUE_CALLBACK()
{
	return m_iUSE_GET_CITY_FOUND_VALUE_CALLBACK;
}

int CvGlobals::getUSE_CANNOT_HANDLE_ACTION_CALLBACK()
{
	return m_iUSE_CANNOT_HANDLE_ACTION_CALLBACK;
}

int CvGlobals::getUSE_CAN_BUILD_CALLBACK()
{
	return m_iUSE_CAN_BUILD_CALLBACK;
}

int CvGlobals::getUSE_CANNOT_TRAIN_CALLBACK()
{
	return m_iUSE_CANNOT_TRAIN_CALLBACK;
}

int CvGlobals::getUSE_CAN_TRAIN_CALLBACK()
{
	return m_iUSE_CAN_TRAIN_CALLBACK;
}

int CvGlobals::getUSE_UNIT_CANNOT_MOVE_INTO_CALLBACK()
{
	return m_iUSE_UNIT_CANNOT_MOVE_INTO_CALLBACK;
}

int CvGlobals::getUSE_USE_CANNOT_SPREAD_RELIGION_CALLBACK()
{
	return m_iUSE_USE_CANNOT_SPREAD_RELIGION_CALLBACK;
}

int CvGlobals::getUSE_FINISH_TEXT_CALLBACK()
{
	return m_iUSE_FINISH_TEXT_CALLBACK;
}

int CvGlobals::getUSE_ON_UNIT_SET_XY_CALLBACK()
{
	return m_iUSE_ON_UNIT_SET_XY_CALLBACK;
}

int CvGlobals::getUSE_ON_UNIT_SELECTED_CALLBACK()
{
	return m_iUSE_ON_UNIT_SELECTED_CALLBACK;
}

int CvGlobals::getUSE_ON_UPDATE_CALLBACK()
{
	return m_iUSE_ON_UPDATE_CALLBACK;
}

int CvGlobals::getUSE_ON_UNIT_CREATED_CALLBACK()
{
	return m_iUSE_ON_UNIT_CREATED_CALLBACK;
}

int CvGlobals::getUSE_ON_UNIT_LOST_CALLBACK()
{
	return m_iUSE_ON_UNIT_LOST_CALLBACK;
}

//FfH: Added by Kael 08/02/2008
int CvGlobals::getUSE_ON_UNIT_MOVE_CALLBACK()
{
	return m_iUSE_ON_UNIT_MOVE_CALLBACK;
}

int CvGlobals::getUSE_PLOT_REVEALED_CALLBACK()
{
	return m_iUSE_PLOT_REVEALED_CALLBACK;
}

int CvGlobals::getUSE_COMBAT_RESULT_CALLBACK()
{
	return m_iUSE_COMBAT_RESULT_CALLBACK;
}
//FfH: End Add

int CvGlobals::getFREE_FAVORED_IMPROVEMENTS_PER_CITY()
{
	return m_iFREE_FAVORED_IMPROVEMENTS_PER_CITY;
}

int CvGlobals::getIMPROVEMENT_COTTAGE()
{
	return m_iIMPROVEMENT_COTTAGE;
}

int CvGlobals::getIMPROVEMENT_FARM()
{
	return m_iIMPROVEMENT_FARM;
}

int CvGlobals::getIMPROVEMENT_GROUNDWATER_WELL()
{
	return m_iIMPROVEMENT_GROUNDWATER_WELL;
}

int CvGlobals::getIMPROVEMENT_PASTURE()
{
	return m_iIMPROVEMENT_PASTURE;
}

int CvGlobals::getIMPROVEMENT_QUARRY()
{
	return m_iIMPROVEMENT_QUARRY;
}

int CvGlobals::getIMPROVEMENT_WATERMILL()
{
	return m_iIMPROVEMENT_WATERMILL;
}

int CvGlobals::getIMPROVEMENT_WINDMILL()
{
	return m_iIMPROVEMENT_WINDMILL;
}

int CvGlobals::getIMPROVEMENT_WORKSHOP()
{
	return m_iIMPROVEMENT_WORKSHOP;
}

int CvGlobals::getFEATURE_HAUNTED_LANDS()
{
	return m_iFEATURE_HAUNTED_LANDS;
}

int CvGlobals::getBUILDINGCLASS_SAGE_DISTRICT()
{
	return m_iBUILDINGCLASS_SAGE_DISTRICT;
}

int CvGlobals::getBUILDINGCLASS_WARRIOR_DISTRICT()
{
	return m_iBUILDINGCLASS_WARRIOR_DISTRICT;
}

int CvGlobals::getBUILDINGCLASS_TEMPLE_DISTRICT()
{
	return m_iBUILDINGCLASS_TEMPLE_DISTRICT;
}

int CvGlobals::getBUILDINGCLASS_BARD_DISTRICT()
{
	return m_iBUILDINGCLASS_BARD_DISTRICT;
}

int CvGlobals::getBUILDINGCLASS_LUXURY_DISTRICT()
{
	return m_iBUILDINGCLASS_LUXURY_DISTRICT;
}

int CvGlobals::getBUILDINGCLASS_FOREIGN_TRADE_DISTRICT()
{
	return m_iBUILDINGCLASS_FOREIGN_TRADE_DISTRICT;
}

int CvGlobals::getBUILDINGCLASS_RESOURCE_DISTRICT()
{
	return m_iBUILDINGCLASS_RESOURCE_DISTRICT;
}

int CvGlobals::getBUILDINGCLASS_HERBALIST()
{
	return m_iBUILDINGCLASS_HERBALIST;
}

int CvGlobals::getBUILDINGCLASS_SCHOLA_ARCANA()
{
	return m_iBUILDINGCLASS_SCHOLA_ARCANA;
}

int CvGlobals::getBUILDINGCLASS_SALON()
{
	return m_iBUILDINGCLASS_SALON;
}

int CvGlobals::getBUILDINGCLASS_NOBLE_DISTRICT()
{
	return m_iBUILDINGCLASS_NOBLE_DISTRICT;
}

int CvGlobals::getBUILDINGCLASS_MERCHANT_DISTRICT()
{
	return m_iBUILDINGCLASS_MERCHANT_DISTRICT;
}

int CvGlobals::getBUILDINGCLASS_PALACE()
{
	return m_iBUILDINGCLASS_PALACE;
}

int CvGlobals::getUNITCLASS_SETTLER()
{
	return m_iUNITCLASS_SETTLER;
}

int CvGlobals::getUNITCLASS_WORKBOAT()
{
	return m_iUNITCLASS_WORKBOAT;
}

int CvGlobals::getUNITCLASS_WORKER()
{
	return m_iUNITCLASS_WORKER;
}

int CvGlobals::getEQUIPMENT_PIECES_OF_BARNAXUS()
{
	return m_iEQUIPMENT_PIECES_OF_BARNAXUS;
}

int CvGlobals::getUNITCOMBAT_ADEPT()
{
	return m_iUNITCOMBAT_ADEPT;
}

int CvGlobals::getSPECIALIST_MERCHANT()
{
	return m_iSPECIALIST_MERCHANT;
}

int CvGlobals::getSPECIALIST_SCIENTIST()
{
	return m_iSPECIALIST_SCIENTIST;
}

int CvGlobals::getSPECIALIST_ARTIST()
{
	return m_iSPECIALIST_ARTIST;
}

int CvGlobals::getSPECIALIST_PRIEST()
{
	return m_iSPECIALIST_PRIEST;
}

int CvGlobals::getSPECIALIST_ENGINEER()
{
	return m_iSPECIALIST_ENGINEER;
}

int CvGlobals::getIMPROVEMENT_MANA_SHRINE()
{
	return m_iIMPROVEMENT_MANA_SHRINE;
}

int CvGlobals::getIMPROVEMENT_TRADE1()
{
	return m_iIMPROVEMENT_TRADE1;
}

int CvGlobals::getIMPROVEMENT_CAMP()
{
	return m_iIMPROVEMENT_CAMP;
}

int CvGlobals::getDEFAULT_SPECIALIST()
{
	return m_iDEFAULT_SPECIALIST;
}

int CvGlobals::getGraphicalDetailPageInRange()
{
	return 14;
//	return std::max(getGameINLINE().getXResolution(), getGameINLINE().getYResolution())/150;
	//return m_iGraphicalDetailPageInRange;
}

int CvGlobals::getMAX_CIV_PLAYERS()
{
	return MAX_CIV_PLAYERS;
}

int CvGlobals::getMAX_PLAYERS()
{
	return MAX_PLAYERS;
}

int CvGlobals::getMAX_CIV_TEAMS()
{
	return MAX_CIV_TEAMS;
}

int CvGlobals::getMAX_TEAMS()
{
	return MAX_TEAMS;
}

int CvGlobals::getBARBARIAN_PLAYER()
{
	return BARBARIAN_PLAYER;
}

int CvGlobals::getBARBARIAN_TEAM()
{
	return BARBARIAN_TEAM;
}

/*************************************************************************************************/
/**	ADDON (MultiBarb) merged Sephi			12/23/08								Xienwolf	**/
/**	adjusted for more barb teams																**/
/**							Adds extra Barbarian Civilizations									**/
/*************************************************************************************************/
int CvGlobals::getWILDMANA_PLAYER()
{
	return WILDMANA_PLAYER;
}

int CvGlobals::getWILDMANA_TEAM()
{
	return WILDMANA_TEAM;
}

int CvGlobals::getANIMAL_PLAYER()
{
	return ANIMAL_PLAYER;
}

int CvGlobals::getANIMAL_TEAM()
{
	return ANIMAL_TEAM;
}

int CvGlobals::getPIRATES_PLAYER()
{
	return PIRATES_PLAYER;
}

int CvGlobals::getPIRATES_TEAM()
{
	return PIRATES_TEAM;
}

int CvGlobals::getWHITEHAND_PLAYER()
{
	return WHITEHAND_PLAYER;
}

int CvGlobals::getWHITEHAND_TEAM()
{
	return WHITEHAND_TEAM;
}

int CvGlobals::getDEVIL_PLAYER() {	return DEVIL_PLAYER;}
int CvGlobals::getDEVIL_TEAM() {   return DEVIL_TEAM; }
int CvGlobals::getORC_PLAYER() {	return ORC_PLAYER;}
int CvGlobals::getORC_TEAM() {   return ORC_TEAM; }

int CvGlobals::getMERCENARIES_PLAYER()
{
	return MERCENARIES_PLAYER;
}

int CvGlobals::getMERCENARIES_TEAM()
{
	return MERCENARIES_TEAM;
}

/**	ADDON (Adventure) Sephi                                                    					**/

int CvGlobals::getMAX_ADVENTURESTEPS()
{
	return MAX_ADVENTURESTEPS;
}

/*************************************************************************************************/
/**	MultiBarb								END													**/
/*************************************************************************************************/

int CvGlobals::getINVALID_PLOT_COORD()
{
	return INVALID_PLOT_COORD;
}

int CvGlobals::getNUM_CITY_PLOTS()
{
	return NUM_CITY_PLOTS;
}

int CvGlobals::getCITY_HOME_PLOT()
{
	return CITY_HOME_PLOT;
}

void CvGlobals::setDLLIFace(CvDLLUtilityIFaceBase* pDll)
{
	m_pDLL = pDll;
}

void CvGlobals::setDLLProfiler(FProfiler* prof)
{
	m_Profiler=prof;
}

FProfiler* CvGlobals::getDLLProfiler()
{
	return m_Profiler;
}

void CvGlobals::enableDLLProfiler(bool bEnable)
{
	m_bDLLProfiler = bEnable;
}

bool CvGlobals::isDLLProfilerEnabled() const
{
	return m_bDLLProfiler;
}

bool CvGlobals::readBuildingInfoArray(FDataStreamBase* pStream)
{
	return readInfoArray(pStream, m_paBuildingInfo, "CvBuildingInfo");
}

void CvGlobals::writeBuildingInfoArray(FDataStreamBase* pStream)
{
	writeInfoArray(pStream, m_paBuildingInfo);
}

bool CvGlobals::readTechInfoArray(FDataStreamBase* pStream)
{
	return readInfoArray(pStream, m_paTechInfo, "CvTechInfo");
}

void CvGlobals::writeTechInfoArray(FDataStreamBase* pStream)
{
	writeInfoArray(pStream, m_paTechInfo);
}

bool CvGlobals::readUnitInfoArray(FDataStreamBase* pStream)
{
	return readInfoArray(pStream, m_paUnitInfo, "CvUnitInfo");
}

void CvGlobals::writeUnitInfoArray(FDataStreamBase* pStream)
{
	writeInfoArray(pStream, m_paUnitInfo);
}

bool CvGlobals::readLeaderHeadInfoArray(FDataStreamBase* pStream)
{
	return readInfoArray(pStream, m_paLeaderHeadInfo, "CvLeaderHeadInfo");
}

void CvGlobals::writeLeaderHeadInfoArray(FDataStreamBase* pStream)
{
	writeInfoArray(pStream, m_paLeaderHeadInfo);
}

bool CvGlobals::readCivilizationInfoArray(FDataStreamBase* pStream)
{
	return readInfoArray(pStream, m_paCivilizationInfo, "CvCivilizationInfo");
}

void CvGlobals::writeCivilizationInfoArray(FDataStreamBase* pStream)
{
	writeInfoArray(pStream, m_paCivilizationInfo);
}

bool CvGlobals::readPromotionInfoArray(FDataStreamBase* pStream)
{
	return readInfoArray(pStream, m_paPromotionInfo, "CvPromotionInfo");
}

void CvGlobals::writePromotionInfoArray(FDataStreamBase* pStream)
{
	writeInfoArray(pStream, m_paPromotionInfo);
}

//FfH: Added by Kael 07/23/2007
bool CvGlobals::readSpellInfoArray(FDataStreamBase* pStream)
{
	return readInfoArray(pStream, m_paSpellInfo, "CvSpellInfo");
}

void CvGlobals::writeSpellInfoArray(FDataStreamBase* pStream)
{
	writeInfoArray(pStream, m_paSpellInfo);
}
//FfH: End Add

/*************************************************************************************************/
/**	BETTER AI (Better Promotions) Sephi                                      					**/
/**																								**/
/**						                                            							**/
/*************************************************************************************************/
bool CvGlobals::readAIPromotionSpecializationInfoArray(FDataStreamBase* pStream)
{
	return readInfoArray(pStream, m_paAIPromotionSpecializationInfo, "CvAIPromotionSpecializationInfo");
}

void CvGlobals::writeAIPromotionSpecializationInfoArray(FDataStreamBase* pStream)
{
	writeInfoArray(pStream, m_paAIPromotionSpecializationInfo);
}
/**	ADDON (Adventures) Sephi                                                      					**/
bool CvGlobals::readAdventureInfoArray(FDataStreamBase* pStream)
{
	return readInfoArray(pStream, m_paAdventureInfo, "CvAdventureInfo");
}

void CvGlobals::writeAdventureInfoArray(FDataStreamBase* pStream)
{
	writeInfoArray(pStream, m_paAdventureInfo);
}

bool CvGlobals::readAdventureStepInfoArray(FDataStreamBase* pStream)
{
	return readInfoArray(pStream, m_paAdventureStepInfo, "CvAdventureStepInfo");
}

void CvGlobals::writeAdventureStepInfoArray(FDataStreamBase* pStream)
{
	writeInfoArray(pStream, m_paAdventureStepInfo);
}

/**	ADDON (Mana Schools) Sephi                                                      					**/
bool CvGlobals::readManaschoolInfoArray(FDataStreamBase* pStream)
{
	return readInfoArray(pStream, m_paManaschoolInfo, "CvManaschoolInfo");
}

void CvGlobals::writeManaschoolInfoArray(FDataStreamBase* pStream)
{
	writeInfoArray(pStream, m_paManaschoolInfo);
}
/**	ADDON (Combatauras) Sephi																	**/
bool CvGlobals::readCombatAuraInfoArray(FDataStreamBase* pStream)
{
	return readInfoArray(pStream, m_paCombatAuraInfo, "CvCombatAuraInfo");
}

void CvGlobals::writeCombatAuraInfoArray(FDataStreamBase* pStream)
{
	writeInfoArray(pStream, m_paCombatAuraInfo);
}

bool CvGlobals::readCombatAuraClassInfoArray(FDataStreamBase* pStream)
{
	return readInfoArray(pStream, m_paCombatAuraClassInfo, "CvCombatAuraClassInfo");
}

void CvGlobals::writeCombatAuraClassInfoArray(FDataStreamBase* pStream)
{
	writeInfoArray(pStream, m_paCombatAuraClassInfo);
}

/*************************************************************************************************/
/**	END	                                        												**/
/*************************************************************************************************/

bool CvGlobals::readDiplomacyInfoArray(FDataStreamBase* pStream)
{
	return readInfoArray(pStream, m_paDiplomacyInfo, "CvDiplomacyInfo");
}

void CvGlobals::writeDiplomacyInfoArray(FDataStreamBase* pStream)
{
	writeInfoArray(pStream, m_paDiplomacyInfo);
}

bool CvGlobals::readCivicInfoArray(FDataStreamBase* pStream)
{
	return readInfoArray(pStream, m_paCivicInfo, "CvCivicInfo");
}

void CvGlobals::writeCivicInfoArray(FDataStreamBase* pStream)
{
	writeInfoArray(pStream, m_paCivicInfo);
}

bool CvGlobals::readHandicapInfoArray(FDataStreamBase* pStream)
{
	return readInfoArray(pStream, m_paHandicapInfo, "CvHandicapInfo");
}

void CvGlobals::writeHandicapInfoArray(FDataStreamBase* pStream)
{
	writeInfoArray(pStream, m_paHandicapInfo);
}

bool CvGlobals::readBonusInfoArray(FDataStreamBase* pStream)
{
	return readInfoArray(pStream, m_paBonusInfo, "CvBonusInfo");
}

void CvGlobals::writeBonusInfoArray(FDataStreamBase* pStream)
{
	writeInfoArray(pStream, m_paBonusInfo);
}

bool CvGlobals::readImprovementInfoArray(FDataStreamBase* pStream)
{
	return readInfoArray(pStream, m_paImprovementInfo, "CvImprovementInfo");
}

void CvGlobals::writeImprovementInfoArray(FDataStreamBase* pStream)
{
	writeInfoArray(pStream, m_paImprovementInfo);
}

bool CvGlobals::readEventInfoArray(FDataStreamBase* pStream)
{
	return readInfoArray(pStream, m_paEventInfo, "CvEventInfo");
}

void CvGlobals::writeEventInfoArray(FDataStreamBase* pStream)
{
	writeInfoArray(pStream, m_paEventInfo);
}

bool CvGlobals::readEventTriggerInfoArray(FDataStreamBase* pStream)
{
	return readInfoArray(pStream, m_paEventTriggerInfo, "CvEventTriggerInfo");
}

void CvGlobals::writeEventTriggerInfoArray(FDataStreamBase* pStream)
{
	writeInfoArray(pStream, m_paEventTriggerInfo);
}


//
// Global Types Hash Map
//

int CvGlobals::getTypesEnum(const char* szType) const
{
	FAssertMsg(szType, "null type string");
	TypesMap::const_iterator it = m_typesMap.find(szType);
	if (it!=m_typesMap.end())
	{
		return it->second;
	}

	FAssertMsg(strcmp(szType, "NONE")==0 || strcmp(szType, "")==0, CvString::format("type %s not found", szType).c_str());
	return -1;
}

void CvGlobals::setTypesEnum(const char* szType, int iEnum)
{
	FAssertMsg(szType, "null type string");
	FAssertMsg(m_typesMap.find(szType)==m_typesMap.end(), "types entry already exists");
	m_typesMap[szType] = iEnum;
}


int CvGlobals::getNUM_ENGINE_DIRTY_BITS() const
{
	return NUM_ENGINE_DIRTY_BITS;
}

int CvGlobals::getNUM_INTERFACE_DIRTY_BITS() const
{
	return NUM_INTERFACE_DIRTY_BITS;
}

int CvGlobals::getNUM_YIELD_TYPES() const
{
	return NUM_YIELD_TYPES;
}

int CvGlobals::getNUM_COMMERCE_TYPES() const
{
	return NUM_COMMERCE_TYPES;
}

int CvGlobals::getNUM_FORCECONTROL_TYPES() const
{
	return NUM_FORCECONTROL_TYPES;
}

int CvGlobals::getNUM_INFOBAR_TYPES() const
{
	return NUM_INFOBAR_TYPES;
}

int CvGlobals::getNUM_HEALTHBAR_TYPES() const
{
	return NUM_HEALTHBAR_TYPES;
}

int CvGlobals::getNUM_CONTROL_TYPES() const
{
	return NUM_CONTROL_TYPES;
}

int CvGlobals::getNUM_LEADERANIM_TYPES() const
{
	return NUM_LEADERANIM_TYPES;
}


void CvGlobals::deleteInfoArrays()
{
	deleteInfoArray(m_paBuildingClassInfo);
	deleteInfoArray(m_paBuildingInfo);
	deleteInfoArray(m_paSpecialBuildingInfo);

	deleteInfoArray(m_paLeaderHeadInfo);
	deleteInfoArray(m_paTraitInfo);
	deleteInfoArray(m_paCivilizationInfo);
	deleteInfoArray(m_paUnitArtStyleTypeInfo);

	deleteInfoArray(m_paVoteSourceInfo);
	deleteInfoArray(m_paHints);
	deleteInfoArray(m_paMainMenus);
/*************************************************************************************************/
/**	TrueModular								05/26/09	Written: Mr. Genie	Imported: Xienwolf	**/
/**																								**/
/**	Properly links Modular modifications to previous elements, and allows partial overwriting	**/
/*************************************************************************************************/
	// Python Modular Loading
	deleteInfoArray(m_paPythonModulesInfo);
	// MLF loading
	m_paModLoadControlVector.clear();
	deleteInfoArray(m_paModLoadControls);
/*************************************************************************************************/
/**	TrueModular								END													**/
/*************************************************************************************************/
	deleteInfoArray(m_paGoodyInfo);
	deleteInfoArray(m_paHandicapInfo);
	deleteInfoArray(m_paGameSpeedInfo);
	deleteInfoArray(m_paTurnTimerInfo);
	deleteInfoArray(m_paVictoryInfo);
	deleteInfoArray(m_paHurryInfo);
	deleteInfoArray(m_paWorldInfo);
	deleteInfoArray(m_paSeaLevelInfo);
	deleteInfoArray(m_paClimateInfo);
	deleteInfoArray(m_paProcessInfo);
	deleteInfoArray(m_paVoteInfo);
	deleteInfoArray(m_paProjectInfo);
	deleteInfoArray(m_paReligionInfo);
	deleteInfoArray(m_paCorporationInfo);
	deleteInfoArray(m_paCommerceInfo);
	deleteInfoArray(m_paEmphasizeInfo);
	deleteInfoArray(m_paUpkeepInfo);
	deleteInfoArray(m_paCultureLevelInfo);

	deleteInfoArray(m_paColorInfo);
	deleteInfoArray(m_paPlayerColorInfo);
	deleteInfoArray(m_paInterfaceModeInfo);
	deleteInfoArray(m_paCameraInfo);
	deleteInfoArray(m_paAdvisorInfo);
	deleteInfoArray(m_paThroneRoomCamera);
	deleteInfoArray(m_paThroneRoomInfo);
	deleteInfoArray(m_paThroneRoomStyleInfo);
	deleteInfoArray(m_paSlideShowInfo);
	deleteInfoArray(m_paSlideShowRandomInfo);
	deleteInfoArray(m_paWorldPickerInfo);
	deleteInfoArray(m_paSpaceShipInfo);

	deleteInfoArray(m_paCivicInfo);
	deleteInfoArray(m_paImprovementInfo);

	deleteInfoArray(m_paRouteInfo);
	deleteInfoArray(m_paRouteModelInfo);
	deleteInfoArray(m_paRiverInfo);
	deleteInfoArray(m_paRiverModelInfo);

	deleteInfoArray(m_paWaterPlaneInfo);
	deleteInfoArray(m_paTerrainPlaneInfo);
	deleteInfoArray(m_paCameraOverlayInfo);

	deleteInfoArray(m_aEraInfo);
	deleteInfoArray(m_paEffectInfo);
	deleteInfoArray(m_paAttachableInfo);

	deleteInfoArray(m_paTechInfo);
	deleteInfoArray(m_paDiplomacyInfo);

	deleteInfoArray(m_paBuildInfo);
	deleteInfoArray(m_paUnitClassInfo);
	deleteInfoArray(m_paUnitInfo);
	deleteInfoArray(m_paSpecialUnitInfo);
	deleteInfoArray(m_paSpecialistInfo);
	deleteInfoArray(m_paActionInfo);
	deleteInfoArray(m_paMissionInfo);
	deleteInfoArray(m_paControlInfo);
	deleteInfoArray(m_paCommandInfo);
	deleteInfoArray(m_paAutomateInfo);
	deleteInfoArray(m_paPromotionInfo);

//FfH: Added by Kael 07/23/2007
	deleteInfoArray(m_paAlignmentInfo);
	deleteInfoArray(m_paDamageTypeInfo);
	deleteInfoArray(m_paSpellInfo);
//FfH: End Add
/*************************************************************************************************/
/**	BETTER AI (Better Promotions) Sephi                                      					**/
/**																								**/
/**						                                            							**/
/*************************************************************************************************/
	deleteInfoArray(m_paAIPromotionSpecializationInfo);
/**	ADDON (Adventures) Sephi                                                      					**/
	deleteInfoArray(m_paAdventureInfo);
	deleteInfoArray(m_paAdventureStepInfo);
	deleteInfoArray(m_paEquipmentCategoryInfo);
	deleteInfoArray(m_paDurabilityInfo);
	deleteInfoArray(m_paDungeonInfo);
	deleteInfoArray(m_paDungeonEventInfo);
	deleteInfoArray(m_paLairResultInfo);
	deleteInfoArray(m_paCityStateInfo);
	deleteInfoArray(m_paPlaneInfo);
	deleteInfoArray(m_paTerrainClassInfo);
	deleteInfoArray(m_paFeatureClassInfo);
	deleteInfoArray(m_paCitySpecializationInfo);
/*************************************************************************************************/
/**	END	                                        												**/
/*************************************************************************************************/
	deleteInfoArray(m_paConceptInfo);
	deleteInfoArray(m_paNewConceptInfo);
/*************************************************************************************************/
/**	ADDON (New Pedia Categories) Sephi                                      					**/
/*************************************************************************************************/
	deleteInfoArray(m_paWildmanaConceptInfo);
	deleteInfoArray(m_paWildmanaGuideInfo);
/*************************************************************************************************/
/**	END	                                        												**/
/*************************************************************************************************/

	deleteInfoArray(m_paCityTabInfo);
	deleteInfoArray(m_paCalendarInfo);
	deleteInfoArray(m_paSeasonInfo);
	deleteInfoArray(m_paMonthInfo);
	deleteInfoArray(m_paDenialInfo);
	deleteInfoArray(m_paInvisibleInfo);
	deleteInfoArray(m_paUnitCombatInfo);
	deleteInfoArray(m_paDomainInfo);
	deleteInfoArray(m_paUnitAIInfos);
/*************************************************************************************************/
/**	ADDON (Better AI) Sephi                                      					**/
/*************************************************************************************************/
	deleteInfoArray(m_paAIGroupInfos);
/*************************************************************************************************/
/**	END	                                        												**/
/*************************************************************************************************/

	deleteInfoArray(m_paAttitudeInfos);
	deleteInfoArray(m_paMemoryInfos);
	deleteInfoArray(m_paGameOptionInfos);
	deleteInfoArray(m_paMPOptionInfos);
	deleteInfoArray(m_paForceControlInfos);
	deleteInfoArray(m_paPlayerOptionInfos);
	deleteInfoArray(m_paGraphicOptionInfos);

	deleteInfoArray(m_paYieldInfo);
	deleteInfoArray(m_paTerrainInfo);
	deleteInfoArray(m_paFeatureInfo);
	deleteInfoArray(m_paBonusClassInfo);
	deleteInfoArray(m_paBonusInfo);
	deleteInfoArray(m_paLandscapeInfo);

	deleteInfoArray(m_paUnitFormationInfo);
	deleteInfoArray(m_paCivicOptionInfo);
	deleteInfoArray(m_paCursorInfo);

	SAFE_DELETE_ARRAY(GC.getEntityEventTypes());
	SAFE_DELETE_ARRAY(GC.getAnimationOperatorTypes());
	SAFE_DELETE_ARRAY(GC.getFunctionTypes());
	SAFE_DELETE_ARRAY(GC.getFlavorTypes());
	SAFE_DELETE_ARRAY(GC.getArtStyleTypes());
	SAFE_DELETE_ARRAY(GC.getCitySizeTypes());
	SAFE_DELETE_ARRAY(GC.getContactTypes());
	SAFE_DELETE_ARRAY(GC.getDiplomacyPowerTypes());
	SAFE_DELETE_ARRAY(GC.getAutomateTypes());
	SAFE_DELETE_ARRAY(GC.getDirectionTypes());
	SAFE_DELETE_ARRAY(GC.getFootstepAudioTypes());
	SAFE_DELETE_ARRAY(GC.getFootstepAudioTags());
	deleteInfoArray(m_paQuestInfo);
	deleteInfoArray(m_paTutorialInfo);

	deleteInfoArray(m_paEventInfo);
	deleteInfoArray(m_paEventTriggerInfo);
	deleteInfoArray(m_paEspionageMissionInfo);

	deleteInfoArray(m_paEntityEventInfo);
	deleteInfoArray(m_paAnimationCategoryInfo);
	deleteInfoArray(m_paAnimationPathInfo);

	clearTypesMap();
	m_aInfoVectors.clear();
}
/*************************************************************************************************/
/**	TrueModular								05/26/09	Written: Mr. Genie	Imported: Xienwolf	**/
/**	LoadedTGA								05/26/09	Written: Mr. Genie	Imported: Xienwolf	**/
/**I would love to know how they figured out what was vital to/used by the EXE, I hope it wasn't**/
/**					just guess and check to narrow it all down in the end						**/
/**	Properly links Modular modifications to previous elements, and allows partial overwriting	**/
/*************************************************************************************************/
void CvGlobals::doResetInfoClasses(int iNumSaveGameVector, std::vector<CvString> m_aszSaveGameVector)
{
	// Store stuff being set by the exe in temp arrays
	StoreExeSettings();

	//delete obsolete InfoClasses
	deleteInfoArrays();

	// reset the ModLoadControlVector
	m_paModLoadControlVector.erase(m_paModLoadControlVector.begin(), m_paModLoadControlVector.end());

	//Load the Savegame vector to the ModLoadControlVector(being used by the enum)
	for ( int i = 0; i < iNumSaveGameVector; i++ )
	{
		m_paModLoadControlVector.push_back(m_aszSaveGameVector[i]);
	}

	//Delete InfoMap Keys
	infoTypeFromStringReset();

	//reload the new infoclasses
	CvXMLLoadUtility XMLLoadUtility;
	XMLLoadUtility.doResetGlobalInfoClasses();

	//Reload Arts with the Current MLF
	CvArtFileMgr ArtFileMgr = ArtFileMgr.GetInstance();
	ArtFileMgr.Reset();

	XMLLoadUtility.doResetInfoClasses();		// Reloads/allocs Art Defines

	// Load stuff being set by the exe from temp arrays
	LoadExeSettings();
}
void CvGlobals::StoreExeSettings()
{
	// Chars from TGA files, CommerceInfo
	m_iStoreExeSettingsCommerceInfo =  new int[NUM_COMMERCE_TYPES];
	for ( int i = 0; i < NUM_COMMERCE_TYPES; i++ )
	{
		m_iStoreExeSettingsCommerceInfo[i] = getCommerceInfo((CommerceTypes)i).getChar();
	}
	// Chars from TGA files, YieldInfo
	m_iStoreExeSettingsYieldInfo =  new int[NUM_YIELD_TYPES];
	for ( int i = 0; i < NUM_YIELD_TYPES; i++ )
	{
		m_iStoreExeSettingsYieldInfo[i] = getYieldInfo((YieldTypes)i).getChar();
	}
	// Chars from TGA files, ReligionInfo
	m_iStoreExeSettingsReligionInfo =  new int[getNumReligionInfos()];
	for ( int i = 0; i < getNumReligionInfos(); i++ )
	{
		m_iStoreExeSettingsReligionInfo[i] = getReligionInfo((ReligionTypes)i).getChar();
	}
	// Chars from TGA files, CorporationInfo
	m_iStoreExeSettingsCorporationInfo =  new int[getNumCorporationInfos()];
	for ( int i = 0; i < getNumCorporationInfos(); i++ )
	{
		m_iStoreExeSettingsCorporationInfo[i] = getCorporationInfo((CorporationTypes)i).getChar();
	}
	// Chars from TGA files, BonusInfo

	m_iStoreExeSettingsBonusInfo =  new int[getNumBonusInfos()];
	for ( int i = 0; i < getNumBonusInfos(); i++ )
	{
		m_iStoreExeSettingsBonusInfo[i] = getBonusInfo((BonusTypes)i).getChar();
	}
}
void CvGlobals::LoadExeSettings()
{
	// Chars from TGA files, CommerceInfo
	for ( int i = 0; i < NUM_COMMERCE_TYPES; i++ )
	{
		getCommerceInfo((CommerceTypes)i).setChar(m_iStoreExeSettingsCommerceInfo[i]);
	}
	SAFE_DELETE_ARRAY(m_iStoreExeSettingsCommerceInfo);
	// Chars from TGA files, YieldInfo
	for ( int i = 0; i < NUM_YIELD_TYPES; i++ )
	{
		getYieldInfo((YieldTypes)i).setChar(m_iStoreExeSettingsYieldInfo[i]);
	}
	SAFE_DELETE_ARRAY(m_iStoreExeSettingsYieldInfo);
	// Chars from TGA files, ReligionInfo
	for ( int i = 0; i < getNumReligionInfos(); i++ )
	{
		getReligionInfo((ReligionTypes)i).setChar(m_iStoreExeSettingsReligionInfo[i]);
	}
	SAFE_DELETE_ARRAY(m_iStoreExeSettingsReligionInfo);
	// Chars from TGA files, CorporationInfo
	for ( int i = 0; i < getNumCorporationInfos(); i++ )
	{
		getCorporationInfo((CorporationTypes)i).setChar(m_iStoreExeSettingsCorporationInfo[i]);
	}
	SAFE_DELETE_ARRAY(m_iStoreExeSettingsCorporationInfo);
	// Chars from TGA files, BonusInfo

	for ( int i = 0; i < getNumBonusInfos(); i++ )
	{
		getBonusInfo((BonusTypes)i).setChar(m_iStoreExeSettingsBonusInfo[i]);
	}
	SAFE_DELETE_ARRAY(m_iStoreExeSettingsBonusInfo);
}
/*************************************************************************************************/
/**	TrueModular								END													**/
/*************************************************************************************************/


//
// Global Infos Hash Map
//

int CvGlobals::getInfoTypeForString(const char* szType, bool hideAssert) const
	{
	FAssertMsg(szType, "null info type string");
	InfosMap::const_iterator it = m_infosMap.find(szType);
	if (it!=m_infosMap.end())
	{
		return it->second;
	}

	if(!hideAssert)
	{
		CvString szError;
		szError.Format("info type %s not found, Current XML file is: %s", szType, GC.getCurrentXMLFile().GetCString());
		bool bError=strcmp(szType, "NONE")==0 || strcmp(szType, "")==0;
		FAssertMsg(strcmp(szType, "NONE")==0 || strcmp(szType, "")==0, szError.c_str());
		//only log if we do not have NONE or ""
		if(!bError)
		{
			//gDLL->logMsg("xml.log", szError);
			gDLL->logMsg("xml.log", szError,false,false);
		}
	}

	return -1;
}

/*************************************************************************************************/
/**	TrueModular								05/26/09	Written: Mr. Genie	Imported: Xienwolf	**/
/**		Debug Assert hidden because sometimes we call it KNOWING that we will trigger it		**/
/**	Properly links Modular modifications to previous elements, and allows partial overwriting	**/
/*************************************************************************************************/
/**								---- Start Original Code ----									**
void CvGlobals::setInfoTypeFromString(const char* szType, int idx)
{
	FAssertMsg(szType, "null info type string");
#ifdef _DEBUG
	InfosMap::const_iterator it = m_infosMap.find(szType);
	int iExisting = (it!=m_infosMap.end()) ? it->second : -1;
	FAssertMsg(iExisting==-1 || iExisting==idx || strcmp(szType, "ERROR")==0, CvString::format("xml info type entry %s already exists", szType).c_str());
#endif
	m_infosMap[szType] = idx;
}
/**								----  End Original Code  ----									**/
void CvGlobals::setInfoTypeFromString(const char* szType, int idx, bool hideAssert)
{
	FAssertMsg(szType, "null info type string");
	m_infosMap[szType] = idx;
}

void CvGlobals::logInfoTypeMap(const char* tagMsg)
{
	CvString szDebugBuffer;
	szDebugBuffer.Format(" === Info Type Map Dump BEGIN: %s ===", tagMsg);
	gDLL->logMsg("CvGlobals_logInfoTypeMap.log", szDebugBuffer.c_str());

	int iCnt = 0;
	std::vector<std::string> vInfoMapKeys;
	for (InfosMap::const_iterator it = m_infosMap.begin(); it != m_infosMap.end(); it++)
	{
		std::string sKey = it->first;
		vInfoMapKeys.push_back(sKey);
	}

	std::sort(vInfoMapKeys.begin(), vInfoMapKeys.end());

	for (std::vector<std::string>::const_iterator it = vInfoMapKeys.begin(); it != vInfoMapKeys.end(); it++)
	{
		std::string sKey = *it;
		int iVal = m_infosMap[sKey];
		szDebugBuffer.Format(" * %i --  %s: %i", iCnt, sKey.c_str(), iVal);
		gDLL->logMsg("CvGlobals_logInfoTypeMap.log", szDebugBuffer.c_str());
		iCnt++;
	}

	szDebugBuffer.Format("Entries in total: %i", iCnt);
	gDLL->logMsg("CvGlobals_logInfoTypeMap.log", szDebugBuffer.c_str());
	szDebugBuffer.Format(" === Info Type Map Dump END: %s ===", tagMsg);
	gDLL->logMsg("CvGlobals_logInfoTypeMap.log", szDebugBuffer.c_str());
}
/*************************************************************************************************/
/**	TrueModular								END													**/
/*************************************************************************************************/

void CvGlobals::infoTypeFromStringReset()
{
	m_infosMap.clear();
}

void CvGlobals::addToInfosVectors(void *infoVector)
{
	std::vector<CvInfoBase *> *infoBaseVector = (std::vector<CvInfoBase *> *) infoVector;
	m_aInfoVectors.push_back(infoBaseVector);
}

void CvGlobals::infosReset()
{
	for(int i=0;i<(int)m_aInfoVectors.size();i++)
	{
		std::vector<CvInfoBase *> *infoBaseVector = m_aInfoVectors[i];
		for(int j=0;j<(int)infoBaseVector->size();j++)
			infoBaseVector->at(j)->reset();
	}
}

int CvGlobals::getNumDirections() const { return NUM_DIRECTION_TYPES; }
int CvGlobals::getNumGameOptions() const { return NUM_GAMEOPTION_TYPES; }
int CvGlobals::getNumMPOptions() const { return NUM_MPOPTION_TYPES; }
int CvGlobals::getNumSpecialOptions() const { return NUM_SPECIALOPTION_TYPES; }
int CvGlobals::getNumGraphicOptions() const { return NUM_GRAPHICOPTION_TYPES; }
int CvGlobals::getNumTradeableItems() const { return NUM_TRADEABLE_ITEMS; }
int CvGlobals::getNumBasicItems() const { return NUM_BASIC_ITEMS; }
int CvGlobals::getNumTradeableHeadings() const { return NUM_TRADEABLE_HEADINGS; }
int CvGlobals::getNumCommandInfos() const { return NUM_COMMAND_TYPES; }
int CvGlobals::getNumControlInfos() const { return NUM_CONTROL_TYPES; }
int CvGlobals::getNumMissionInfos() const { return NUM_MISSION_TYPES; }
int CvGlobals::getNumPlayerOptionInfos() const { return NUM_PLAYEROPTION_TYPES; }
int CvGlobals::getMaxNumSymbols() const { return MAX_NUM_SYMBOLS; }
int CvGlobals::getNumGraphicLevels() const { return NUM_GRAPHICLEVELS; }
int CvGlobals::getNumGlobeLayers() const { return NUM_GLOBE_LAYER_TYPES; }


//
// non-inline versions
//
CvMap& CvGlobals::getMap() { return *m_map; }
CvGameAI& CvGlobals::getGame() { return *m_game; }
CvGameAI *CvGlobals::getGamePointer(){ return m_game; }

TimeMeasure& CvGlobals::getTimeMeasure() { return *m_TimeMeasure; }
/*************************************************************************************************/
/**	ADDON (WINAMP) merged Sephi																	**/
/*************************************************************************************************/
CvWinamp& CvGlobals::getWinamp() { return *m_Winamp; }
/*************************************************************************************************/
/**	ADDON (WINAMP)						END														**/
/*************************************************************************************************/

/*************************************************************************************************/
/**	Viewport C2C				                                   								**/
/*************************************************************************************************/
/**
CvViewport& CvGlobals::getCurrentViewport()
{
	return *getMapINLINE().getCurrentViewport();
}
/*************************************************************************************************/
/**	END Viewport C2C			                                        						**/
/*************************************************************************************************/		  		

int CvGlobals::getMaxCivPlayers() const
{
	return MAX_CIV_PLAYERS;
}

bool CvGlobals::IsGraphicsInitialized() const { return m_bGraphicsInitialized;}
void CvGlobals::SetGraphicsInitialized(bool bVal) { m_bGraphicsInitialized = bVal;}
void CvGlobals::setInterface(CvInterface* pVal) { m_interface = pVal; }
void CvGlobals::setDiplomacyScreen(CvDiplomacyScreen* pVal) { m_diplomacyScreen = pVal; }
void CvGlobals::setMPDiplomacyScreen(CMPDiplomacyScreen* pVal) { m_mpDiplomacyScreen = pVal; }
void CvGlobals::setMessageQueue(CMessageQueue* pVal) { m_messageQueue = pVal; }
void CvGlobals::setHotJoinMessageQueue(CMessageQueue* pVal) { m_hotJoinMsgQueue = pVal; }
void CvGlobals::setMessageControl(CMessageControl* pVal) { m_messageControl = pVal; }
void CvGlobals::setSetupData(CvSetupData* pVal) { m_setupData = pVal; }
void CvGlobals::setMessageCodeTranslator(CvMessageCodeTranslator* pVal) { m_messageCodes = pVal; }
void CvGlobals::setDropMgr(CvDropMgr* pVal) { m_dropMgr = pVal; }
void CvGlobals::setPortal(CvPortal* pVal) { m_portal = pVal; }
void CvGlobals::setStatsReport(CvStatsReporter* pVal) { m_statsReporter = pVal; }
void CvGlobals::setPathFinder(FAStar* pVal) { m_pathFinder = pVal; }
void CvGlobals::setInterfacePathFinder(FAStar* pVal) { m_interfacePathFinder = pVal; }
void CvGlobals::setStepFinder(FAStar* pVal) { m_stepFinder = pVal; }
void CvGlobals::setRouteFinder(FAStar* pVal) { m_routeFinder = pVal; }
void CvGlobals::setBorderFinder(FAStar* pVal) { m_borderFinder = pVal; }
void CvGlobals::setAreaFinder(FAStar* pVal) { m_areaFinder = pVal; }
void CvGlobals::setPlotGroupFinder(FAStar* pVal) { m_plotGroupFinder = pVal; }
void CvGlobals::setStepUnitFinder(FAStar* pVal) { m_stepUnitFinder = pVal; }
CvDLLUtilityIFaceBase* CvGlobals::getDLLIFaceNonInl() { return m_pDLL; }
