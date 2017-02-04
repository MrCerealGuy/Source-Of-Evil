/*

 * PROJECT     : Source of Evil Runtime Library v1.0 
 * PROGRAMMER  : Andreas Zahnleiter (Mafuba)    

*/

#ifndef SOE_SCREEN_H
#define SOE_SCREEN_H

#include "SoE_Clist.h"

// structure for videomodes
typedef struct 
{
    DWORD Width;
    DWORD Height;
    DWORD BPP;
    void* NextMode;
} SOE_VIDEOMODESSTRUCT;

// fix for those strange DDRAW header files where it is missing
#ifndef MAX_DDDEVICEID_STRING
	#define MAX_DDDEVICEID_STRING 512
#endif

// structure for direct draw drivers
struct SOE_DRIVERSTRUCT
{
	LPGUID	lpGUID;
	GUID	guid;
	char	lpName[MAX_DDDEVICEID_STRING];
	char	lpDesc[MAX_DDDEVICEID_STRING];
	DDCAPS	DDcaps;
    DDCAPS  HELcaps;

#if DIRECTDRAW_VERSION >= SOE_DDVER
	HMONITOR hMonitor;
#endif

	SOE_DRIVERSTRUCT *NextDriver;
};
		
// callback function for video mode enumeration, implemented in ddutils.cpp
HRESULT CALLBACK EnumDisplayModesCallback( SOE_LPDDSURFACEDESC lpDDSurfaceDesc, LPVOID lpContext  );

// callback function for devices enumeration, implemented in ddutils.cpp
#if DIRECTDRAW_VERSION >= SOE_DDVER
BOOL WINAPI DDEnumCallback( GUID FAR *lpGUID, LPSTR lpDesc, LPSTR lpName, LPVOID lpContext, HMONITOR hm );
#else
BOOL WINAPI DDEnumCallback( GUID FAR *lpGUID, LPSTR lpDesc, LPSTR lpName, LPVOID lpContext );
#endif

// Forward declaration of SoESurface needed by the SoEScreen class
class SOEAPI SoESurface;

class SOEAPI SoEScreen
{
public:
	SoEScreen();
	virtual ~SoEScreen();

	HRESULT CreateFullScreen(void* hWnd, DWORD Width, DWORD Height, DWORD BPP, BOOL bVGA = FALSE);
	HRESULT CreateWindowed(void* hWnd, int Width, int Height);
	HRESULT LoadBitmap(const char* szFilename);
	HRESULT LoadPalette(const char* szFilename);
	HRESULT Fill(DWORD FillColor);
	HRESULT Restore(void);
	void    RestoreAllSurfaces(void);
	HRESULT Flip(BOOL VSync=TRUE, BOOL FlipWithStretch=TRUE, BOOL displayFPS = FALSE);
	HRESULT SetColor(int col, int r, int g, int b);
	HRESULT GetColor(int col, int* r, int* g, int* b);
	HRESULT SetPalette(int Start, int Count, LPPALETTEENTRY lpPE);
	HRESULT GetPalette(int Start, int Count, LPPALETTEENTRY lpPE);
	HRESULT FillPalette(int r, int g, int b);
	HRESULT GreyScale(void);
	HRESULT FadeIn(int delay, LPPALETTEENTRY lpPE);
	HRESULT FadeOut(int delay);
	HRESULT FadeTo(int r, int g, int b, int delay);
	int     GetWidth(void)  { return m_dwPixelWidth; }
	int     GetHeight(void) { return m_dwPixelHeight; }
	int     GetBPP(void)    { return m_BPP; }
	SoESurface* GetFront(void) { return m_lpDDSFront; }
	SoESurface* GetBack(void) { return m_lpDDSBack; }
	LPDIRECTDRAWPALETTE GetPalette(void) { return m_lpDDPalette; }
    DWORD   GetNumberOfVideoModes( void );
    BOOL    GetVideoModeInformation( DWORD Index, DWORD* Width , DWORD* Height , DWORD* BPP );
    BOOL    CheckIfVideoModeExists( DWORD Width, DWORD Height , DWORD BPP );
	HRESULT WaitForVerticalBlank( void );
	void    CreateBackBufferInVRAM( BOOL Flag ) { m_BackBufferInVRAM = Flag; };
	void    SetTripleBuffering( BOOL Flag ) { m_TripleBuffering = Flag; }
	BOOL    GetTripleBuffering( void ) { return m_TripleBuffering; }
    HRESULT ChangeVideoMode(DWORD Width, DWORD Height,DWORD BPP, BOOL bVGA = FALSE);
    DWORD   MakeColor( DWORD Color , SoESurface* Surface = NULL );
	void    AddSurfaceToList( SoESurface* Surface );
	void    ReleaseAllSurfaces( void );
	void    ReleaseSurfaceFromList( SoESurface* Surface );
	HRESULT SelectDriver( DWORD Index );
	HRESULT FadeToBlack(WORD duration);
	HRESULT FadeToSource(WORD duration);
    BOOL    IsFullScreen(void) {return m_bFullScreen;}
    void*   GetWindowHandle(void) {return m_hWnd;}
	SOE_LPDIRECTDRAW GetDD(void) { return m_lpDD; }
    HRESULT SaveAllSurfaces(void);
    HRESULT ReloadAllSurfaces(void);
    LONG    GetNextId(void);

protected:
    void    InitSoEScreen();
	HRESULT EnumVideoModes( void );
	HRESULT EnumDrivers( void );
    void    SetSurfaceScreen(SoESurface* surface);
    void    SetSurfacePixelFormat(SoESurface* surface);
    void    SetSurfaceWidth(SoESurface* surface, int w);
    void    SetSurfaceHeight(SoESurface* surface, int h);
	void    SetSurfaceDDSPointer(SoESurface* pSoES, SOE_LPDIRECTDRAWSURFACE lpdds);

public:  // private:

	SOE_LPDIRECTDRAW          m_lpDD;              // the DirectDraw object
    SOE_LPDIRECTDRAWSURFACE   m_ZBuffer;           // Zbuffer surface pointer
	SoESurface*            m_lpDDSFront;           // a SoESurface object for the front buffer
	SoESurface*            m_lpDDSBack;            // a SoESurface object for the back buffer
	LPDIRECTDRAWPALETTE    m_lpDDPalette;          // the DirectDraw palette object
	LPDIRECTDRAWCLIPPER    m_lpClipper;            // the DirectDraw clipper, used for window mode only
	DWORD                  m_dwPixelWidth;         // the screen width, in pixels
	DWORD                  m_dwPixelHeight;        // the screen height, in pixels
	DWORD                  m_BPP;                  // bits per pixel (colour depth)
	void*                  m_hWnd;                 // pointer to the HWND handle of the window
	BOOL                   m_bFullScreen;          // is the program running in fullscreen mode?
    DWORD                  m_NumberOfVideoModes;   // number of video modes the video adapter supports
    SOE_VIDEOMODESSTRUCT*  m_VideoModes;           // linked list with all video modes informations
	BOOL                   m_TripleBuffering;      // is triple buffering enabled ?
	BOOL                   m_BackBufferInVRAM;     // Should the back buffer be placed in video memory?
    DWORD                  m_NumberOfDrivers;      // number of grafikcards installed
    SOE_DRIVERSTRUCT*      m_Drivers;              // linked list with grafikcard informations
    DWORD                  m_SelectedDriver;       // Which driver will be used for creating DirectDraw
	DWORD                  m_frameCount;           // A count of how many times you have called Flip()
	DWORD                  m_accumulatedFrameTime; // How long since the last time you called Flip()
	DWORD                  m_frameStartTime;       // The time when the frame started
	LONG                   m_fps;                  // The frames per second rate
    SoECList<SoESurface>   m_SurfaceList;          // Keeps track of all SoESurfaces created
    static LONG            m_lastId;
    
};

#endif
