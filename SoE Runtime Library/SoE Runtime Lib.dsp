# Microsoft Developer Studio Project File - Name="SoE RunLib" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=SoE RunLib - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "SoE Runtime Lib.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "SoE Runtime Lib.mak" CFG="SoE RunLib - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "SoE RunLib - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "SoE RunLib - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "SoE RunLib - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\SoE Temp\RunLibRelease"
# PROP Intermediate_Dir "..\SoE Temp\RunLibRelease"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /I ".\inc" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:".\lib\SoE.lib"

!ELSEIF  "$(CFG)" == "SoE RunLib - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\SoE Temp\RunLibDebug"
# PROP Intermediate_Dir "..\SoE Temp\RunLibDebug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I ".\inc" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /D "CDX_LOG" /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:".\lib\SoED.lib"

!ENDIF 

# Begin Target

# Name "SoE RunLib - Win32 Release"
# Name "SoE RunLib - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\src\_template.cpp
# End Source File
# Begin Source File

SOURCE=.\src\SoE_Ddutils.cpp
# End Source File
# Begin Source File

SOURCE=.\src\SoE_Debug.cpp
# End Source File
# Begin Source File

SOURCE=.\src\SoE_Dxerrors.cpp
# End Source File
# Begin Source File

SOURCE=.\src\SoE_File.cpp
# End Source File
# Begin Source File

SOURCE=.\src\SoE_GUIButton.cpp
# End Source File
# Begin Source File

SOURCE=.\src\SoE_GUIImage.cpp
# End Source File
# Begin Source File

SOURCE=.\src\SoE_GUIList.cpp
# End Source File
# Begin Source File

SOURCE=.\src\SoE_GUIListEntry.cpp
# End Source File
# Begin Source File

SOURCE=.\src\SoE_GUISystem.cpp
# End Source File
# Begin Source File

SOURCE=.\src\SoE_GUIWindow.cpp
# End Source File
# Begin Source File

SOURCE=.\src\SoE_Image.cpp
# End Source File
# Begin Source File

SOURCE=.\src\SoE_Imagebase.cpp
# End Source File
# Begin Source File

SOURCE=.\src\SoE_ImageBMP.cpp
# End Source File
# Begin Source File

SOURCE=.\src\SoE_ImagePCX.cpp
# End Source File
# Begin Source File

SOURCE=.\src\SoE_ImagePSD.cpp
# End Source File
# Begin Source File

SOURCE=.\src\SoE_ImageTGA.cpp
# End Source File
# Begin Source File

SOURCE=.\src\SoE_Input.cpp
# End Source File
# Begin Source File

SOURCE=.\src\SoE_Iso.cpp
# End Source File
# Begin Source File

SOURCE=.\src\SoE_Layer.cpp
# End Source File
# Begin Source File

SOURCE=.\src\SoE_Link.cpp
# End Source File
# Begin Source File

SOURCE=.\src\SoE_Log.cpp
# End Source File
# Begin Source File

SOURCE=.\src\SoE_Map.cpp
# End Source File
# Begin Source File

SOURCE=.\src\SoE_MapCell.cpp
# End Source File
# Begin Source File

SOURCE=.\src\SoE_Midi.cpp
# End Source File
# Begin Source File

SOURCE=.\src\SoE_Mouse.cpp
# End Source File
# Begin Source File

SOURCE=.\src\SoE_Movie.cpp
# End Source File
# Begin Source File

SOURCE=.\src\SoE_Music.cpp
# End Source File
# Begin Source File

SOURCE=.\src\SoE_MusicCD.cpp
# End Source File
# Begin Source File

SOURCE=.\src\SoE_Resource.cpp
# End Source File
# Begin Source File

SOURCE=.\src\SoE_Screen.cpp
# End Source File
# Begin Source File

SOURCE=.\src\SoE_Sound.cpp
# End Source File
# Begin Source File

SOURCE=.\src\SoE_SoundBuffer.cpp
# End Source File
# Begin Source File

SOURCE=.\src\SoE_SpecMap.cpp
# End Source File
# Begin Source File

SOURCE=.\src\SoE_Sprite.cpp
# End Source File
# Begin Source File

SOURCE=.\src\SoE_SpriteList.cpp
# End Source File
# Begin Source File

SOURCE=.\src\SoE_String.cpp
# End Source File
# Begin Source File

SOURCE=.\src\SoE_Surface.cpp
# End Source File
# Begin Source File

SOURCE=.\src\SoE_Tile.cpp
# End Source File
# Begin Source File

SOURCE=.\src\SoE_Window.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\inc\SoE.h
# End Source File
# Begin Source File

SOURCE=.\inc\SoE_Clist.h
# End Source File
# Begin Source File

SOURCE=.\inc\SoE_Debug.h
# End Source File
# Begin Source File

SOURCE=.\inc\SoE_File.h
# End Source File
# Begin Source File

SOURCE=.\inc\SoE_GUI.h
# End Source File
# Begin Source File

SOURCE=.\inc\SoE_Image.h
# End Source File
# Begin Source File

SOURCE=.\inc\SoE_Imagebase.h
# End Source File
# Begin Source File

SOURCE=.\inc\SoE_ImageBMP.h
# End Source File
# Begin Source File

SOURCE=.\inc\SoE_ImagePCX.h
# End Source File
# Begin Source File

SOURCE=.\inc\SoE_ImagePSD.h
# End Source File
# Begin Source File

SOURCE=.\inc\SoE_ImageTGA.h
# End Source File
# Begin Source File

SOURCE=.\inc\SoE_Input.h
# End Source File
# Begin Source File

SOURCE=.\inc\SoE_Layer.h
# End Source File
# Begin Source File

SOURCE=.\inc\SoE_Link.h
# End Source File
# Begin Source File

SOURCE=.\inc\SoE_Log.h
# End Source File
# Begin Source File

SOURCE=.\inc\SoE_Map.h
# End Source File
# Begin Source File

SOURCE=.\inc\SoE_Midi.h
# End Source File
# Begin Source File

SOURCE=.\inc\SoE_Mouse.h
# End Source File
# Begin Source File

SOURCE=.\inc\SoE_Movie.h
# End Source File
# Begin Source File

SOURCE=.\inc\SoE_Music.h
# End Source File
# Begin Source File

SOURCE=.\inc\SoE_Resource.h
# End Source File
# Begin Source File

SOURCE=.\inc\SoE_Screen.h
# End Source File
# Begin Source File

SOURCE=.\inc\SoE_Sound.h
# End Source File
# Begin Source File

SOURCE=.\inc\SoE_SpecMap.h
# End Source File
# Begin Source File

SOURCE=.\inc\SoE_Sprite.h
# End Source File
# Begin Source File

SOURCE=.\inc\SoE_String.h
# End Source File
# Begin Source File

SOURCE=.\inc\SoE_Structs.h
# End Source File
# Begin Source File

SOURCE=.\inc\SoE_Surface.h
# End Source File
# Begin Source File

SOURCE=.\inc\SoE_Template.h
# End Source File
# Begin Source File

SOURCE=.\inc\SoE_Tile.h
# End Source File
# Begin Source File

SOURCE=.\inc\SoE_Types.h
# End Source File
# Begin Source File

SOURCE=.\inc\SoE_Vector.h
# End Source File
# Begin Source File

SOURCE=.\inc\SoE_Window.h
# End Source File
# End Group
# End Target
# End Project
