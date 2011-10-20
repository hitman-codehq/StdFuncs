# Microsoft Developer Studio Project File - Name="StdFuncs" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=StdFuncs - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "StdFuncs.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "StdFuncs.mak" CFG="StdFuncs - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "StdFuncs - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "StdFuncs - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "StdFuncs - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /W4 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD BASE RSC /l 0x809 /d "NDEBUG"
# ADD RSC /l 0x809 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "StdFuncs - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /W4 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD BASE RSC /l 0x809 /d "_DEBUG"
# ADD RSC /l 0x809 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "StdFuncs - Win32 Release"
# Name "StdFuncs - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\Args.cpp
# End Source File
# Begin Source File

SOURCE=.\BaUtils.cpp
# End Source File
# Begin Source File

SOURCE=.\Dir.cpp
# End Source File
# Begin Source File

SOURCE=.\File.cpp
# End Source File
# Begin Source File

SOURCE=.\Lex.cpp
# End Source File
# Begin Source File

SOURCE=.\MungWall.cpp
# End Source File
# Begin Source File

SOURCE=.\StdApplication.cpp
# End Source File
# Begin Source File

SOURCE=.\StdClipboard.cpp
# End Source File
# Begin Source File

SOURCE=.\StdDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\StdFileRequester.cpp
# End Source File
# Begin Source File

SOURCE=.\StdFont.cpp
# End Source File
# Begin Source File

SOURCE=.\StdGadgetLayout.cpp
# End Source File
# Begin Source File

SOURCE=.\StdGadgets.cpp
# End Source File
# Begin Source File

SOURCE=.\StdGadgetSlider.cpp
# End Source File
# Begin Source File

SOURCE=.\StdGadgetStatusBar.cpp
# End Source File
# Begin Source File

SOURCE=.\StdImage.cpp
# End Source File
# Begin Source File

SOURCE=.\StdTextFile.cpp
# End Source File
# Begin Source File

SOURCE=.\StdWildcard.cpp
# End Source File
# Begin Source File

SOURCE=.\StdWindow.cpp
# End Source File
# Begin Source File

SOURCE=.\Test.cpp
# End Source File
# Begin Source File

SOURCE=.\Time.cpp
# End Source File
# Begin Source File

SOURCE=.\Utils.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\Args.h
# End Source File
# Begin Source File

SOURCE=.\BaUtils.h
# End Source File
# Begin Source File

SOURCE=.\Dir.h
# End Source File
# Begin Source File

SOURCE=.\File.h
# End Source File
# Begin Source File

SOURCE=.\Lex.h
# End Source File
# Begin Source File

SOURCE=.\MungWall.h
# End Source File
# Begin Source File

SOURCE=.\StdApplication.h
# End Source File
# Begin Source File

SOURCE=.\StdClipboard.h
# End Source File
# Begin Source File

SOURCE=.\StdDialog.h
# End Source File
# Begin Source File

SOURCE=.\StdFileRequester.h
# End Source File
# Begin Source File

SOURCE=.\StdFont.h
# End Source File
# Begin Source File

SOURCE=.\StdFuncs.h
# End Source File
# Begin Source File

SOURCE=.\StdGadgets.h
# End Source File
# Begin Source File

SOURCE=.\StdImage.h
# End Source File
# Begin Source File

SOURCE=.\StdList.h
# End Source File
# Begin Source File

SOURCE=.\StdTextFile.h
# End Source File
# Begin Source File

SOURCE=.\StdWildcard.h
# End Source File
# Begin Source File

SOURCE=.\StdWindow.h
# End Source File
# Begin Source File

SOURCE=.\Test.h
# End Source File
# Begin Source File

SOURCE=.\Time.h
# End Source File
# Begin Source File

SOURCE=.\Utils.h
# End Source File
# End Group
# End Target
# End Project
