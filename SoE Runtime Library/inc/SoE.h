/*

 * PROJECT     : Source of Evil Runtime Library v1.0 
 * PROGRAMMER  : Andreas Zahnleiter (Mafuba)    

*/

#ifndef SOE_H
#define SOE_H

#if defined(_SOE_BUILDDLL) && defined(_SOE_USEDLL)
#error Sorry, you cannot use _SOE_BUILDDLL & _SOE_USEDLL together
#endif

// this is initially defined as nothing
#define SOEAPI

// this should be specified to make SoE as a DLL
#ifdef _SOE_BUILDDLL
#undef SOEAPI
#define SOEAPI __declspec(dllexport)
#endif

// the following should be specified if the user wants to
// use SoE.dll.  Otherwise chaos ensues when you try to delete objects
#ifdef _SOE_USEDLL
#undef SOEAPI
#define SOEAPI __declspec(dllimport)
#endif

// SoE currently supports DX7
#define DIRECTDRAW_VERSION	      0x0700
#define DIRECTINPUT_VERSION	      0x0700
#define DIRECTSOUND_VERSION	      0x0700
#define SOE_LPDIRECTDRAWSURFACE   LPDIRECTDRAWSURFACE7
#define SOE_LPDIRECTDRAW          LPDIRECTDRAW7
#define SOE_DDSURFACEDESC         DDSURFACEDESC2
#define SOE_LPDDSURFACEDESC       LPDDSURFACEDESC2
#define SOE_DDSCAPS               DDSCAPS2
#define SOE_LPDIRECTDRAWPALETTE   LPDIRECTDRAWPALETTE

// SoE Currently support DirectX 7
#define SOE_DDVER		0x0700
#define SOE_DIVER		0x0700
#define SOE_DSVER		0x0700

// System includes
#include <windows.h>
#include <windowsx.h>
#include <mmsystem.h>
#include <stdio.h>

// DirectX includes
#include "..\..\SoE DirectX\Inc\Ddraw.h"
#include "..\..\SoE DirectX\Inc\Dinput.h"
#include "..\..\SoE DirectX\Inc\Dsound.h"
#include "..\..\SoE DirectX\Inc\Dplay.h"
#include "..\..\SoE DirectX\Inc\Dplobby.h"

// SoE Includes
#include "SoE_Types.h"
#include "SoE_Log.h"
#include "SoE_Structs.h"
#include "SoE_Surface.h"
#include "SoE_Screen.h"

#ifdef SOEINCLUDEALL
	#include "SoE_File.h"
	#include "SoE_Surface.h"
	#include "SoE_Layer.h"
	#include "SoE_Tile.h"
	#include "SoE_Map.h"
	#include "SoE_Sprite.h"
	#include "SoE_Sound.h"
	#include "SoE_Music.h"
	#include "SoE_Input.h"
	#include "SoE_Link.h"
	#include "SoE_Resource.h"
	#include "SoE_SpecMap.h"
	//#include "SoE_Packer.h"
	#include "SoE_Midi.h"
	#include "SoE_Image.h"
	#include "SoE_Vector.h"
	#include "SoE_Window.h"
	#include "SoE_String.h"
	#include "SoE_Mouse.h"
	#include "SoE_Movie.h"
	#include "SoE_GUI.h"
#endif


//////////////////////////////////////////////////////////////////////////////////
// Globals
//////////////////////////////////////////////////////////////////////////////////
extern "C"
{

	// DDUTILS Functions
  extern SOEAPI SOE_LPDIRECTDRAWPALETTE DDLoadPalette(SOE_LPDIRECTDRAW pdd, LPCSTR szBitmap);
	extern SOEAPI SOE_LPDIRECTDRAWSURFACE DDLoadBitmap(SOE_LPDIRECTDRAW pdd, LPCSTR szBitmap, int dx, int dy, BOOL memoryType);
	extern SOEAPI SOE_LPDIRECTDRAWSURFACE DDLoadSizeBitmap(SOE_LPDIRECTDRAW pdd, LPCSTR szBitmap, int *dx, int *dy, BOOL memoryType);

	// Why no extern SOEAPI on this one???
	SOE_LPDIRECTDRAWSURFACE DDCreateSurface(SOE_LPDIRECTDRAW pdd, DWORD width, DWORD height, SOE_LPDDSURFACEDESC ddsd);

	// Why no SOEAPI on these ???
	extern HRESULT       DDReLoadBitmap(SOE_LPDIRECTDRAWSURFACE pdds, LPCSTR szBitmap);
	extern HRESULT       DDCopyBitmap(SOE_LPDIRECTDRAWSURFACE pdds, HBITMAP hbm, int x, int y, int dx, int dy);
	extern DWORD         DDColorMatch(SOE_LPDIRECTDRAWSURFACE pdds, COLORREF rgb);
	extern HRESULT       DDSetColorKey(SOE_LPDIRECTDRAWSURFACE pdds, COLORREF rgb);
  extern BOOL          GetRGBFormat(SOE_LPDIRECTDRAWSURFACE Surface, RGBFORMAT* rgb);
	extern BOOL __cdecl  IsMMX(void);

	// DirectX Errors
	extern SOEAPI void DDError(HRESULT hErr, void* hWnd, LPCSTR szFile, int iLine);
	extern SOEAPI void SoEError( SoEScreen *Screen, char *str, ...);
	extern SOEAPI void DSError(HRESULT hErr, void* hWnd);
	extern SOEAPI char* DXErrorToString( HRESULT hErr );
}

//////////////////////////////////////////////////////////////////////////////////
// MACROS
//////////////////////////////////////////////////////////////////////////////////
#define SAFEDELETE(x) if(x != NULL) { delete x; x = NULL; }
#define DELETEARRAY(x) if(x != NULL) { delete [] x; x = NULL; }
#define RELEASE(x) if(x != NULL) { x->Release(); x = NULL; }
#define ODS(a) OutputDebugString(a)

#endif
