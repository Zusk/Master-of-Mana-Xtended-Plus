####  Civilization 4 SDK Makefile  ####
#### Copyright 2010 Danny Daemonic ####
#######################################

#### Paths ####
TOOLKIT=F:/Program Files (x86)/Microsoft Visual C++ Toolkit 2003
##PSDK=F:/Program Files/Microsoft SDKs/Windows/v6.0A
PSDK=F:/Program Files (x86)/WindowsSDK
## Uncomment to have newly compiled dlls copied to your mod's Assets directory
YOURMOD=F:\Games\Sid Meier's Civilization 4\Beyond the Sword\Mods\Master of Mana Workspace

#### Tools ####
CC="$(TOOLKIT)/bin/cl.exe"
CPP="$(TOOLKIT)/bin/cl.exe"
LD="$(TOOLKIT)/bin/link.exe"
RC="$(PSDK)/bin/rc.exe"
## Uncomment to build dependencies using fastdep
#FD="$(MAKEDIR)/bin/fastdep.exe"

#### BLACKLIST ####
## Uncomment to block CvTextScreen (accidentally included by Firaxis)
BLACKLIST=CvTextScreens

#### You shouldn't need to modify anything beyond this point ####
#################################################################

#### Target Files ####
Debug_BIN=Debug\CvGameCoreDLL.dll
#Debug_RESOURCE=Debug\CvGameCoreDLL.res
Debug_STATICLIB=Debug\CvGameCoreDLL.lib
Debug_LIBDEF=Debug\CvGameCoreDLL.def
Debug_PCH=Debug\CvGameCoreDLL.pch
Debug_PDB=Debug\CvGameCoreDLL.pdb
Debug_OTHER=Debug\CvGameCoreDLL.exp Debug\CvGameCoreDLL.ilk
Release_BIN=Release\CvGameCoreDLL.dll
#Release_RESOURCE=Release\CvGameCoreDLL.res
Release_STATICLIB=Release\CvGameCoreDLL.lib
Release_LIBDEF=Release\CvGameCoreDLL.def
Release_PCH=Release\CvGameCoreDLL.pch
Release_PDB=Release\CvGameCoreDLL.pdb
Release_OTHER=Release\CvGameCoreDLL.exp

#### CFLAGS ####
#GLOBAL_CFLAGS=/GR /Gy /W3 /EHsc /Gd /DWIN32 /D_WINDOWS /D_USRDLL /DCVGAMECOREDLL_EXPORTS /Yu"CvGameCoreDLL.h"
#Debug_CFLAGS=/MD /Zi /Od /D_DEBUG /RTC1 /Fp"$(Debug_PCH)" $(GLOBAL_CFLAGS)
GLOBAL_CFLAGS=/GR /Gy /W3 /EHsc /Gd /DWIN32 /D_WINDOWS /D_USRDLL /DCVGAMECOREDLL_EXPORTS /Yu"CvGameCoreDLL.h"
Debug_CFLAGS=/MD /Zi /Od /D_DEBUG /RTC1 /Fp"$(Debug_PCH)" $(GLOBAL_CFLAGS)
Release_CFLAGS=/MD /O2 /Oy /Oi /G7 /DNDEBUG /DFINAL_RELEASE /Fp"$(Release_PCH)" $(GLOBAL_CFLAGS)

#### LDFLAGS ####
GLOBAL_LDFLAGS=/DLL /NOLOGO /SUBSYSTEM:WINDOWS /LARGEADDRESSAWARE /TLBID:1
Debug_LDFLAGS=/INCREMENTAL /DEBUG /PDB:"$(Debug_PDB)" /IMPLIB:"$(Debug_STATICLIB)" $(GLOBAL_LDFLAGS)
Release_LDFLAGS=/INCREMENTAL:NO /OPT:REF /OPT:ICF /PDB:"$(Release_PDB)" $(GLOBAL_LDFLAGS)

#### INCLUDES ####
GLOBAL_INCS=/I"$(TOOLKIT)/include" /I"$(PSDK)/Include"
PROJECT_INCS=/IBoost-1.32.0/include /IPython24/include /Itbb/include /ISource/AI /ISource
Debug_INCS=$(PROJECT_INCS) $(GLOBAL_INCS)
Release_INCS=$(PROJECT_INCS) $(GLOBAL_INCS)

#### LIBS ####
GLOBAL_LIBS=/LIBPATH:"$(TOOLKIT)/lib" /LIBPATH:"$(PSDK)/Lib" winmm.lib user32.lib
PROJECT_LIBS=/LIBPATH:Python24/libs /LIBPATH:boost-1.32.0/libs/ boost_python-vc71-mt-1_32.lib 
Debug_LIBS=$(PROJECT_LIBS) $(GLOBAL_LIBS) msvcprt.lib #/NODEFAULTLIB:msvcprtd.lib
Release_LIBS=$(PROJECT_LIBS) $(GLOBAL_LIBS)

#### Objects ####
Debug_LINKOBJS=$(Debug_OBJS)
Release_LINKOBJS=$(Release_OBJS)

SOURCES=*.cpp Source\AI\*.cpp Source\GUI\*.cpp


#### Auto SOURCES/OBJS ####
!IF [echo SOURCES= \> sources.mk] == 0 && \
    [for %i in ($(SOURCES)) do @echo. "%i" \>> sources.mk] == 0 && \
    [echo.>> sources.mk] == 0 && \
    [echo Debug_OBJS= \>> sources.mk] == 0 && \
    [for /f "delims=." %i in ('dir /b $(SOURCES)') DO @echo. Debug\%i.obj \>> sources.mk] == 0 && \
    [echo.>> sources.mk] == 0 && \
    [echo Release_OBJS= \>> sources.mk] == 0 && \
    [for /f "delims=." %i in ('dir /b $(SOURCES)') DO @echo. Release\%i.obj \>> sources.mk] == 0 && \
    [echo.>> sources.mk] == 0
!INCLUDE sources.mk
!IF [del sources.mk]
!ENDIF
!ENDIF

#### Targets ####
#################

.PHONY: all clean Debug_clean Release_clean Debug Release

all: Debug Release

clean: Debug_clean Release_clean 

Debug_clean:
	@for %i in ($(Debug_BIN) $(Debug_STATICLIB) $(Debug_LIBDEF) \
		Debug\*.obj Debug\*.@ $(Debug_RESOURCE) \
		$(Debug_PCH) $(Debug_PDB) $(Debug_OTHER)) DO @if exist "%i" del "%i"

Release_clean:
	@for %i in ($(Release_BIN) $(Release_STATICLIB) $(Release_LIBDEF) \
		Release\*.obj Release\*.@ $(Release_RESOURCE) \
		$(Release_PCH) $(Release_PDB) $(Release_OTHER)) DO @if exist "%i" del "%i"

Debug: Debug_DIR Debug_unfinished $(Debug_PCH) $(Debug_BIN)
!IFDEF YOURMOD
	-copy "$(Debug_BIN)" "$(YOURMOD)\Assets\."
!ENDIF

Release: Release_DIR Release_unfinished $(Release_PCH) $(Release_BIN)
!IFDEF YOURMOD
	-copy "$(Release_BIN)" "$(YOURMOD)\Assets\."
!ENDIF

Debug_DIR:
	-@if not exist "Debug\." mkdir "Debug"

Release_DIR:
	-@if not exist "Release\." mkdir "Release"

Debug_unfinished:
	@echo.>Debug\unfinished.@
	@for /f "delims=@" %i in ('dir /b Debug\*.@') DO \
		@if exist "Debug\%i" del "Debug\%i"
	@for /f %i in ('dir /b Debug\*.@') DO \
		@if exist "Debug\%i" del "Debug\%i"

Release_unfinished:
	@echo.>Release\unfinished.@
	@for /f "delims=@" %i in ('dir /b Release\*.@') DO \
		@if exist "Release\%i" del "Release\%i"
	@for /f %i in ('dir /b Release\*.@') DO \
		@if exist "Release\%i" del "Release\%i"

$(Debug_BIN): $(Debug_LINKOBJS) $(Debug_RESOURCE)
	$(LD) /out:$(Debug_BIN) $(Debug_LDFLAGS) $(Debug_LIBS) $(Debug_LINKOBJS) $(Debug_RESOURCE)

$(Release_BIN): $(Release_LINKOBJS) $(Release_RESOURCE)
	$(LD) /out:$(Release_BIN) $(Release_LDFLAGS) $(Release_LIBS) $(Release_LINKOBJS) $(Release_RESOURCE)

.cpp{Debug}.obj:
	@echo.>"$*.obj.@"
    $(CPP) /nologo $(Debug_CFLAGS) $(Debug_INCS) /Fo$*.obj /c $<
	@del "$*.obj.@"

.cpp{Release}.obj:
	@echo.>"$*.obj.@"
    $(CPP) /nologo $(Release_CFLAGS) $(Release_INCS) /Fo$*.obj /c $<
	@del "$*.obj.@"

$(Debug_PCH) Debug\_precompile.obj:
	@echo.>"$(Debug_PCH).@"
	@echo.>"Debug\_precompile.obj.@"
	$(CPP) /nologo $(Debug_CFLAGS) $(Debug_INCS) /YcCvGameCoreDLL.h /Fo"Debug\_precompile.obj" /c _precompile.cpp
	@del "$(Debug_PCH).@"
	@del "Debug\_precompile.obj.@"

$(Release_PCH) Release\_precompile.obj:
	@echo.>"$(Release_PCH).@"
	@echo.>"Release\_precompile.obj.@"
    $(CPP) /nologo $(Release_CFLAGS) $(Release_INCS) /YcCvGameCoreDLL.h /Fo"Release\_precompile.obj" /c _precompile.cpp
	@del "$(Release_PCH).@"
	@del "Release\_precompile.obj.@"

.rc{Debug}.res:
	@echo.>"$*.res.@"
	$(RC) /Fo$@ $(Debug_INCS) $<
	@del "$*.res.@"

.rc{Release}.res:
	@echo.>"$*.res.@"
	$(RC) /Fo$@ $(Release_INCS) $<
	@del "$*.res.@"

!IFDEF BLACKLIST

Debug\$(BLACKLIST).obj: $(BLACKLIST).cpp
	@echo.>"$*.obj.@"
	@echo.>"$*-dummy.cpp"
	$(CPP) /nologo $(Debug_CFLAGS) $(Debug_INCS) /Y- /Fo$@ /c "$*-dummy.cpp"
	@del "$*-dummy.cpp"
	@del "$*.obj.@"

Release\$(BLACKLIST).obj: $(BLACKLIST).cpp
	echo.>"$*.obj.@"
	echo.>"$*-dummy.cpp"
	$(CPP) /nologo $(Release_CFLAGS) $(Release_INCS) /Y- /Fo$@ /c "$*-dummy.cpp"
	del "$*-dummy.cpp"
	del "$*.obj.@"
!ENDIF

!IFDEF FD
!IF [$(FD) --objectextension=obj -q -O Debug $(SOURCES) > depends] == 0 && \
    [$(FD) --objectextension=obj -q -O Release $(SOURCES) >> depends] == 0
!ENDIF
!ENDIF

!IF EXIST(depends)
!INCLUDE depends
!ENDIF
