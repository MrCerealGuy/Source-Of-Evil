/*

 * PROJECT     : Source of Evil Runtime Library v1.0 
 * PROGRAMMER  : Andreas Zahnleiter (Mafuba)    

*/

#include "SoE.h"
#include "SoE_Screen.h"
#include "SoE_Surface.h"

LONG SoEScreen::m_lastId = 0;

void SoEScreen::InitSoEScreen()
{
	HRESULT   rval;
	
	SOELOG("START: SoEScreen::InitSoEScreen");
	SOELOG("Initialize member vars");
	
	// initialize member variables
	m_lpDD					= NULL;
    m_ZBuffer				= NULL;
	m_lpDDSFront			= NULL;
	m_lpDDSBack				= NULL;
	m_lpDDPalette			= NULL;
	m_lpClipper				= NULL;
    m_dwPixelWidth			= 0;
	m_dwPixelHeight			= 0;
    m_BPP					= 0;
	m_hWnd					= 0;
	m_bFullScreen			= FALSE;
    m_NumberOfVideoModes    = 0;
    m_VideoModes            = NULL;
	m_TripleBuffering		= FALSE;
	m_BackBufferInVRAM		= TRUE;
    m_NumberOfDrivers       = 0;
	m_Drivers               = NULL;
	m_SelectedDriver		= 0;
	m_frameCount			= 0;
	m_accumulatedFrameTime	= 0;
	m_frameStartTime		= timeGetTime();
	m_fps					= 0;
	
	// create direct draw object
#if DIRECTDRAW_VERSION >= SOE_DDVER
	SOELOG("SOE_DDVER: 0x%X (%i)", SOE_DDVER, SOE_DDVER);
	SOELOG("DIRECTDRAW_VERSION: 0x%X (i)", DIRECTDRAW_VERSION, DIRECTDRAW_VERSION);
	
	SOELOG("Attempting a DirectDrawCreateEx");
	rval = DirectDrawCreateEx(NULL, (VOID**)&m_lpDD, IID_IDirectDraw7, NULL);
	if (FAILED(rval)) 
    {
        SOELOG("Failed to create an IID_IDirectDraw7 Interface!");
    }
	
#else
	SOELOG("SOE_DDVER: 0x%X (%i)", SOE_DDVER, SOE_DDVER);
	SOELOG("DIRECTDRAW_VERSION: 0x%X (%i)", DIRECTDRAW_VERSION, DIRECTDRAW_VERSION);
	
	SOELOG("Attempting a DirectDrawCreate");
	
	rval = DirectDrawCreate(NULL, &m_lpDD, NULL);
	if (FAILED(rval))
    {
        SOELOG("Failed to create an IID_IDirectDraw Interface!");
    }
#endif
	
	// Enumerate the drivers
	EnumDrivers();
	
    // Enumerate the Video Modes
	EnumVideoModes();
	
	SOELOG("END: SoEScreen::InitSoEScreen");
}

SoEScreen::SoEScreen()
{
	SOELOG("START: SoEScreen::SoEScreen()");
	
	InitSoEScreen();
	
	SOELOG("END: SoEScreen::SoEScreen()");
}

SoEScreen::~SoEScreen(void)
{
    SOE_VIDEOMODESSTRUCT* Modes;
    SOE_VIDEOMODESSTRUCT* Modes2;
	SOE_DRIVERSTRUCT* Drivers;
    SOE_DRIVERSTRUCT* Drivers2;
	
    SOELOG("START: SoEScreen::~SoEScreen");
	SOELOG("Modes Ptr: %i", m_VideoModes);
	
	// detstroy linked list with video modes informations
    Modes = m_VideoModes;
    do
    {
        Modes2 = Modes;
        Modes  = (SOE_VIDEOMODESSTRUCT *)Modes->NextMode;
		if (Modes2 != NULL)
			delete Modes2;
    }
    while (Modes != NULL);
	
	SOELOG("Drivers Ptr: %i", m_Drivers);
	
	// destroy linked list with direct draw drivers informations
    Drivers = m_Drivers;
    do
    {
        Drivers2 = Drivers;
        Drivers  = Drivers->NextDriver;
		if (Drivers2 != NULL)
			delete Drivers2;
    }
    while (Drivers != NULL);
	
	SOELOG("Restore Display");
	if (m_lpDD != NULL) 
		m_lpDD->RestoreDisplayMode();
	
	SOELOG("Freeing all surfaces");
	ReleaseAllSurfaces();
	
    // Only release the back buffer if we are in windowed mode. Otherwise
    // releasing the front buffer will take care of it.
	SOELOG("Delete back buffer");
	if ((GetBack() != NULL) &&(IsFullScreen() == FALSE))
		SAFEDELETE(m_lpDDSBack);
	
	SOELOG("Delete front buffer");
	if (GetFront() != NULL)
		SAFEDELETE(m_lpDDSFront);
	
	SOELOG("Start releasing objects");
	
	SOELOG("Release zBuffer");
	RELEASE(m_ZBuffer);
	
	SOELOG("Release clipper");
	RELEASE(m_lpClipper);
	
	SOELOG("Release Palette");
	RELEASE(m_lpDDPalette);
	
	SOELOG("Release DirectDraw");
	RELEASE(m_lpDD);
	
	SOELOG("END: SoEScreen::~SoEScreen");
}

HRESULT SoEScreen::CreateWindowed(void *hWnd, int Width, int Height)
{
	HRESULT rval;
    HRESULT rc = 0;
	DWORD dwFlags;
	HDC hDC;
	DWORD width, height;
    RECT rect;
	SOE_DDSURFACEDESC ddsd;
	SOE_LPDIRECTDRAWSURFACE lpdds;
	
	SOELOG("START: SoEScreen::CreateWindowed(hwnd:%i, width:%i, height:%i)", hWnd, Width, Height);
	
    // See if we have a valid DirectDraw Interface
    if (GetDD() == NULL)
    {
        SOELOG("DirectDraw Interface pointer is NULL");
        return -1;
    }
	
    // If the front buffer is valid then exit
    if (m_lpDDSFront != NULL)
    {
        SOELOG("Attempted to recreate windowed and the front buffer is not NULL");
        return -1;
    }
	
	SOELOG("Set member variables");
	
	m_bFullScreen   = FALSE;
	m_dwPixelWidth  = Width;
	m_dwPixelHeight = Height;
	m_hWnd          = hWnd;
	
	SOELOG("Get Device Caps");
	
	hDC = GetDC(NULL);
    if (hDC != NULL)
    {
		m_BPP = GetDeviceCaps(hDC, PLANES) * GetDeviceCaps(hDC, BITSPIXEL);
		ReleaseDC(NULL, hDC);
    }
    else
    {
        return -1;
    }
	
	SOELOG("Set cooperative level");
	
	dwFlags = DDSCL_NORMAL;
	
	rval = m_lpDD->SetCooperativeLevel((HWND)hWnd, dwFlags);
    if (FAILED(rval)) 
        return -2;
	
	ZeroMemory(&ddsd, sizeof(ddsd));
	ddsd.dwSize  = sizeof(ddsd);
	ddsd.dwFlags = DDSD_CAPS;
	ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
	
	SOELOG("Create front buffer");
	m_lpDDSFront = new SoESurface();
	
	SOELOG("Create back buffer");
	m_lpDDSBack = new SoESurface();
	
	SOELOG("create front buffer surface");
	rval = GetDD()->CreateSurface(&ddsd, &lpdds, NULL);
    SetSurfaceDDSPointer(GetFront(), lpdds);
    if (FAILED(rval)) 
    {
        SAFEDELETE(m_lpDDSBack);
		SAFEDELETE(m_lpDDSFront);
        return -4;
    }
	
	
	ddsd.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT;    
	
	// Check if backbuffer is to be created in vram or sys ram
	if (m_BackBufferInVRAM == TRUE)
		ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_VIDEOMEMORY;
	else
		ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY;
	
	ddsd.dwWidth  = Width;
	ddsd.dwHeight = Height;
	
	SOELOG("Create back buffer surface");
	rval = GetDD()->CreateSurface(&ddsd, &lpdds, NULL);
    SetSurfaceDDSPointer(GetBack(), lpdds);
	if (FAILED(rval)) 
    {
		// if backbuffer creation in vram fails, try in sysram
		if (m_BackBufferInVRAM == TRUE)
		{
			ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY;
			SOELOG("Try creating back buffer in system ram");
			rval = GetDD()->CreateSurface(&ddsd, &lpdds, NULL);
            SetSurfaceDDSPointer(GetBack(), lpdds);
			if (FAILED(rval))
            {
                SAFEDELETE(m_lpDDSBack);
				SAFEDELETE(m_lpDDSFront);
                return -8;
            }
		}
		else
        {
            SAFEDELETE(m_lpDDSBack);
			SAFEDELETE(m_lpDDSFront);
			
            return -8;
        }
    }
	
	SOELOG("Create clipper object");
	rval = m_lpDD->CreateClipper(0, &m_lpClipper, NULL);
    if (FAILED(rval)) 
        rc |= -16;
	
	SOELOG("Set Hwnd (%i) for clipper", hWnd);
	rval = m_lpClipper->SetHWnd(0, (HWND)hWnd);
    if (FAILED(rval)) 
        rc |= -32;
	
	SOELOG("Assign clipper object to front surface");
	rval = GetFront()->GetDDS()->SetClipper(m_lpClipper);
    if (FAILED(rval)) 
        rc |= -64;
	
	SOELOG("Set params for front buffer");
	SOELOG("W: %i, H: %i", m_dwPixelWidth, m_dwPixelHeight);
	
	hDC    = GetDC(NULL);
	width  = GetDeviceCaps(hDC, HORZRES);
	height = GetDeviceCaps(hDC, VERTRES);
	ReleaseDC(NULL, hDC);
	
	SetSurfaceWidth(GetFront(), width);
	SetSurfaceHeight(GetFront(), height);
    rect.top    = 0;
    rect.left   = 0;
    rect.right  = width;
    rect.bottom = height;
	
	SetSurfaceScreen(GetFront());
	GetFront()->SetClipRect(&rect);
	SOELOG("Get RGB Format for front buffer");
	SetSurfacePixelFormat(GetFront());
    GetFront()->FunctionMapper();
	
	SOELOG("Set params for back buffer");
	
	SetSurfaceWidth(GetBack(), m_dwPixelWidth);
	SetSurfaceHeight(GetBack(), m_dwPixelHeight);
    rect.top    = 0;
    rect.left   = 0;
    rect.right  = m_dwPixelWidth;
    rect.bottom = m_dwPixelHeight;
	
	SetSurfaceScreen(GetBack());
	GetBack()->SetClipRect(&rect);
	SOELOG("Get RGB Format for back buffer");
	SetSurfacePixelFormat(GetBack());
    GetBack()->FunctionMapper();
	
	SOELOG("Load default palette");
	// create a default palette
	m_lpDDPalette = DDLoadPalette(m_lpDD, NULL);
	SOELOG("Set palette for front surface");
	GetFront()->GetDDS()->SetPalette(m_lpDDPalette);
	
	SOELOG("END: SoEScreen::CreateWindowed");
	
	return rc;
}

HRESULT SoEScreen::CreateFullScreen(void *hWnd, DWORD Width, DWORD Height, DWORD BPP, BOOL bVGA)
{
	HRESULT rval;
    HRESULT rc = 0;
	DWORD dwFlags;
    RECT rect;
	SOE_DDSURFACEDESC ddsd;
	SOE_DDSCAPS ddscaps;
	SOE_LPDIRECTDRAWSURFACE lpdds;
	
	SOELOG("START: SoEScreen::CreateFullScreen(hwnd:%i, width:%i, height:%i, bpp:%i, bVGA:%i)", hWnd, Width, Height, BPP, bVGA);
	
    // See if we have a valid DirectDraw Interface
    if (GetDD() == NULL)
    {
        SOELOG("DirectDraw Interface pointer is NULL");
        return -1;
    }
	
    // If the front buffer is valid then exit
    if (m_lpDDSFront != NULL)
    {
        SOELOG("Attempted to recreate in Fullscreen and the front buffer is not NULL");
        return -1;
    }
	
	SOELOG("Set member variables");
	
	m_bFullScreen   = TRUE;
	m_dwPixelWidth  = Width;
	m_dwPixelHeight = Height;
	m_BPP           = BPP;
	m_hWnd          = hWnd;
	
	SOELOG("Set cooperative level");
	
	dwFlags = DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN | DDSCL_ALLOWREBOOT | DDSCL_ALLOWMODEX;
	
	rval = m_lpDD->SetCooperativeLevel((HWND)hWnd, dwFlags);
    if (FAILED(rval)) 
        return -2;
	
	SOELOG("Set video mode");
#if DIRECTDRAW_VERSION >= SOE_DDVER
	if (bVGA) 
        rval = m_lpDD->SetDisplayMode(m_dwPixelWidth, m_dwPixelHeight, m_BPP, 0, DDSDM_STANDARDVGAMODE);
	else 
        rval = m_lpDD->SetDisplayMode(m_dwPixelWidth, m_dwPixelHeight, m_BPP, 0, 0);
#else
	rval = m_lpDD->SetDisplayMode(m_dwPixelWidth, m_dwPixelHeight, m_BPP);
#endif
	
    if (FAILED(rval)) 
        return -4;
	
	SOELOG("Create front buffer");
	m_lpDDSFront = new SoESurface;
	
	SOELOG("Create back buffer");
	m_lpDDSBack = new SoESurface;
	
	ZeroMemory(&ddsd, sizeof(ddsd));
	ddsd.dwSize  = sizeof(ddsd);
	ddsd.dwFlags = DDSD_CAPS | DDSD_BACKBUFFERCOUNT;
	if (m_BackBufferInVRAM == FALSE)
		ddsd.ddsCaps.dwCaps = DDSCAPS_3DDEVICE | DDSCAPS_PRIMARYSURFACE | DDSCAPS_FLIP | DDSCAPS_COMPLEX | DDSCAPS_SYSTEMMEMORY;
	else
		ddsd.ddsCaps.dwCaps = DDSCAPS_3DDEVICE | DDSCAPS_PRIMARYSURFACE | DDSCAPS_FLIP | DDSCAPS_COMPLEX | DDSCAPS_VIDEOMEMORY;
	
	// Try to get 2 backbuffers, if it fails only one
	if (m_TripleBuffering == TRUE)
	{
		ddsd.dwBackBufferCount = 2;
		
		SOELOG("create front buffer surface (for triple buffering)");
		rval = m_lpDD->CreateSurface(&ddsd, &lpdds, NULL);
        SetSurfaceDDSPointer(GetFront(), lpdds);
		if (FAILED(rval)) 
		{
			m_TripleBuffering      = FALSE;
			ddsd.dwBackBufferCount = 1;
			
			SOELOG("create front buffer surface (can't use triple buffering)");
			rval = m_lpDD->CreateSurface(&ddsd, &lpdds, NULL);
            SetSurfaceDDSPointer(GetFront(), lpdds);
			
			// if creation of 1 backbuffer in video memory fails, try one in system memory
			if (FAILED(rval)) 
            {
				if (m_BackBufferInVRAM == TRUE)
				{
					ddsd.ddsCaps.dwCaps = DDSCAPS_3DDEVICE | DDSCAPS_PRIMARYSURFACE | DDSCAPS_FLIP | DDSCAPS_COMPLEX | DDSCAPS_SYSTEMMEMORY;
					SOELOG("create front buffer surface (in system ram, can't use triple buffering)");
                    rval = m_lpDD->CreateSurface(&ddsd, &lpdds, NULL);
                    SetSurfaceDDSPointer(GetFront(), lpdds);
                    if (FAILED(rval)) 
                    {
                        SAFEDELETE(m_lpDDSBack);
						SAFEDELETE(m_lpDDSFront);
						
                        return -8;
                    }
				}
				else
                {
                    SAFEDELETE(m_lpDDSBack);
					SAFEDELETE(m_lpDDSFront);
                    return -16;
                }
            }
		}
	}
	else
	{
		ddsd.dwBackBufferCount = 1;
		
		SOELOG("create front buffer surface");
		rval = m_lpDD->CreateSurface(&ddsd, &lpdds, NULL);
        SetSurfaceDDSPointer(GetFront(), lpdds);
		// if creation of 1 backbuffer in video memory fails, try one in system memory
		if (FAILED(rval))
        {
			if (m_BackBufferInVRAM == TRUE)
			{
				ddsd.ddsCaps.dwCaps = DDSCAPS_3DDEVICE | DDSCAPS_PRIMARYSURFACE | DDSCAPS_FLIP | DDSCAPS_COMPLEX | DDSCAPS_SYSTEMMEMORY;
				SOELOG("create front buffer surface (in system ram)");
				rval = m_lpDD->CreateSurface(&ddsd, &lpdds, NULL);
                SetSurfaceDDSPointer(GetFront(), lpdds);
                if (FAILED(rval)) 
                {
                    SAFEDELETE(m_lpDDSBack);
					SAFEDELETE(m_lpDDSFront);
                    return -8;
                }
			}
			else
            {
                SAFEDELETE(m_lpDDSBack);
				SAFEDELETE(m_lpDDSFront);
				
                return -16;
            }
        }
	}
	
    ZeroMemory(&ddscaps, sizeof(ddscaps));
	ddscaps.dwCaps = DDSCAPS_BACKBUFFER;
	
	SOELOG("Get the attached back buffer surface");
	rval = GetFront()->GetDDS()->GetAttachedSurface(&ddscaps, &lpdds);
    SetSurfaceDDSPointer(GetBack(), lpdds);
    if (FAILED(rval))
    {
        SAFEDELETE(m_lpDDSBack);
		SAFEDELETE(m_lpDDSFront);
		
        return -32;
    }
	
	SOELOG("Set params for front buffer");
	
	SOELOG("W: %i, H: %i", m_dwPixelWidth, m_dwPixelHeight);
	
	SetSurfaceWidth(GetFront(), m_dwPixelWidth);
	SetSurfaceHeight(GetFront(), m_dwPixelHeight);
    rect.top    = 0;
    rect.left   = 0;
    rect.right  = m_dwPixelWidth;
    rect.bottom = m_dwPixelHeight;
	
	SetSurfaceScreen(GetFront());
	GetFront()->SetClipRect(&rect);
	SOELOG("Get RGB Format for front buffer");
	SetSurfacePixelFormat(GetFront());
    GetFront()->FunctionMapper();
	
	SOELOG("Set params for back buffer");
	
	SetSurfaceWidth(GetBack(), m_dwPixelWidth);
	SetSurfaceHeight(GetBack(), m_dwPixelHeight);
	
	SetSurfaceScreen(GetBack());
	GetBack()->SetClipRect(&rect);
	SOELOG("Get RGB Format for back buffer");
	SetSurfacePixelFormat(GetBack());
    GetBack()->FunctionMapper();
	
	SOELOG("Load default palette");
	// create a default palette
	m_lpDDPalette = DDLoadPalette(m_lpDD, NULL);
	SOELOG("Set palette for front surface");
	GetFront()->GetDDS()->SetPalette(m_lpDDPalette);
	
	SOELOG("END: SoEScreen::CreateFullScreen");
	
	return 0;
}

HRESULT SoEScreen::Flip(BOOL VSync, BOOL FlipWithStretch, BOOL displayFPS)
{
	HRESULT rval;
	CHAR str[32];
	DWORD now;
    RECT Window, rSrc;
	POINT pt;
	
	SOELOG2("START: SoEScreen::Flip");
	
	// Calculate the time it took to build the frame and display it on the back buffer
	now = timeGetTime();
	m_accumulatedFrameTime += now - m_frameStartTime;
	m_frameStartTime = now;
	
	if (m_accumulatedFrameTime > 1000)
	{
		m_fps = (LONG)(((float)m_frameCount /(float)m_accumulatedFrameTime) * 1000);
		m_accumulatedFrameTime = 0;
		m_frameCount = 0;
	}
	
    // Increment the framecount
	m_frameCount++;
	
    // Show the FPS if displayFPS is set to TRUE.
	if (displayFPS)
	{
		// Print the FPS on the back buffer
		memset(str, '\0', 32);
		sprintf(str, "FPS = %d", m_fps);
		
		GetBack()->GetDC();
		GetBack()->TextXY(10, 20, 0x0000FFF0, str);
		GetBack()->ReleaseDC();
	}
	
	if (m_bFullScreen)
	{
		SOELOG2("do a fullscreen flip");
		
		if (VSync == TRUE)
			rval = GetFront()->GetDDS()->Flip(NULL, DDFLIP_WAIT);
		else
#if DIRECTDRAW_VERSION >= SOE_DDVER
			rval = GetFront()->GetDDS()->Flip(NULL, DDFLIP_NOVSYNC);
#else
		rval = GetFront()->GetDDS()->Flip(NULL, DDFLIP_WAIT);
#endif
		
		if (rval == DDERR_SURFACELOST) 
            rval = Restore();
	}
	else
	{
		SOELOG2("windowed mode flip");
		
		GetClientRect((HWND)m_hWnd, &Window);
		pt.x = pt.y = 0;
		ClientToScreen((HWND)m_hWnd, &pt);
		OffsetRect(&Window, pt.x, pt.y);
		
		if (VSync == TRUE)
			WaitForVerticalBlank();
		
		// stretch the surface to fit into the client window rect
		if (FlipWithStretch)
        {
			rval = GetFront()->GetDDS()->Blt(&Window, GetBack()->GetDDS(), NULL, DDBLT_WAIT, NULL);
        }
		else
		{
			// size down the Src rect so that it fits into the window without stretching
			// note that anything beyond the RECT is clipped out of the display, but this
			// also allows it to run like a bat out of hades.  Stretches are slow.
			rSrc.left   = 0; 
            rSrc.top    = 0;
			rSrc.right  = Window.right - Window.left;
			rSrc.bottom = Window.bottom - Window.top;
			rval = GetFront()->GetDDS()->Blt(&Window, GetBack()->GetDDS(), &rSrc, DDBLT_WAIT, NULL);
		}
		
		if (rval == DDERR_SURFACELOST) 
            rval = Restore();
	}
	
	SOELOG2("END: SoEScreen::Flip");
	
	return rval;
}

HRESULT SoEScreen::Fill(DWORD FillColor)
{
	DDBLTFX ddBltFx;
    HRESULT rval;
	
	ddBltFx.dwSize = sizeof(DDBLTFX);
	ddBltFx.dwFillColor = FillColor;
	rval = GetBack()->GetDDS()->Blt(NULL, NULL, NULL, DDBLT_WAIT | DDBLT_COLORFILL, &ddBltFx);
	
    return rval;
}

HRESULT SoEScreen::LoadBitmap(const char* szFilename)
{
    HRESULT rval;
	
	SOELOG("START: SoEScreen::LoadBitmap(szFilename: %s)", szFilename);
	
	if (szFilename == NULL) 
        return -1;
	
	SOELOG("load the bitmap into the back buffer");
	
	rval = DDReLoadBitmap(GetBack()->GetDDS(), szFilename);
	
	SOELOG("END: SoEScreen::LoadBitmap");
	
	return rval;
}

HRESULT SoEScreen::LoadPalette(const char* szFilename)
{
	SOELOG("START: SoEScreen::LoadBitmap(szFilename: %s)", szFilename);
	
	if (szFilename == NULL) 
        return -1;
	
	SOELOG("Load the palette");
	
	m_lpDDPalette = DDLoadPalette(m_lpDD, szFilename);
	if (m_lpDDPalette == NULL) 
        return -2;
	
	SOELOG("Assign the palette to the front surface");
	GetFront()->GetDDS()->SetPalette(m_lpDDPalette);
	
	SOELOG("END: SoEScreen::LoadBitmap");
	
	return 0;
}

void SoEScreen::RestoreAllSurfaces(void)
{
	SoESurface* pSurf = NULL; 

	for (int i = 0; i != m_SurfaceList.GetCount(); i++)
	{
		if ((pSurf = m_SurfaceList.Get(i)) != NULL)
			pSurf->Restore();
	}
}

HRESULT SoEScreen::Restore(void)
{
    HRESULT rval = 0;
	SOELOG("START: SoEScreen::Restore");
	
	if (m_bFullScreen)
	{
		// In fullscreen mode the surfaces are chained together and
		// the DX restore function will restore both surfaces.
		if (GetFront()->GetDDS() != NULL && FAILED(GetFront()->GetDDS()->IsLost()))
		{
			rval = GetFront()->GetDDS()->Restore();
		}
	}
	else
	{
		// In Windowed mode the too surfaces are not related therefore 
		// we need to check both of them.
		if (GetFront()->GetDDS() != NULL && FAILED(GetFront()->GetDDS()->IsLost()))
		{
			rval = GetFront()->GetDDS()->Restore();
		}
		
		if (GetBack()->GetDDS() != NULL && FAILED(GetBack()->GetDDS()->IsLost()))
		{
			rval = GetBack()->GetDDS()->Restore();
		}
	}
	
	SOELOG("END: SoEScreen::Restore");
	
    return rval;
}

HRESULT SoEScreen::SetColor(int col, int r, int g, int b)
{
    HRESULT rval;
	PALETTEENTRY pe[1];
	
	SOELOG("START: SoEScreen::SetColor(col:%i, r:%i, g:%i, b:%i)", col, r, g, b);
	SOELOG("Get the color entry");
	
	rval = m_lpDDPalette->GetEntries(0, col, 1, pe);
    if (FAILED(rval))
        return rval;
	
	pe[0].peRed   = r;
	pe[0].peGreen = g;
	pe[0].peBlue  = b;
	
	SOELOG("Set the color entry");
	
	rval = m_lpDDPalette->SetEntries(0, col, 1, pe);
	
	SOELOG("END: SoEScreen::SetColor");
	
    return rval;
}

HRESULT SoEScreen::GetColor(int col, int *r, int *g, int *b)
{
    HRESULT rval;
	PALETTEENTRY pe[1];
	
	SOELOG("START: SoEScreen::GetColor(col:%i, r:%i, g:%i, b:%i)", col, r, g, b);
	SOELOG("Get the color entry");
	
	rval = m_lpDDPalette->GetEntries(0, col, 1, pe);
	
    if (FAILED(rval))
        return rval;
	
	*r = pe[0].peRed;
	*g = pe[0].peGreen;
	*b = pe[0].peBlue;
	
	SOELOG("r:%i, g:%i, b:%i)", pe[0].peRed, pe[0].peGreen, pe[0].peBlue);
	SOELOG("END: SoEScreen::GetColor");
	
    return rval;
}

HRESULT SoEScreen::SetPalette(int Start, int Count, LPPALETTEENTRY lpPE)
{
    HRESULT rval;
	
	SOELOG("START: SoEScreen::SetPalette(start:%i, count:%i, lpPE:%i)", Start, Count, lpPE);
	
	rval = m_lpDDPalette->SetEntries(0, Start, Count, lpPE);
	
	SOELOG("END: SoEScreen::SetPalette");
	
    return rval;
}

HRESULT SoEScreen::GetPalette(int Start, int Count, LPPALETTEENTRY lpPE)
{
    HRESULT rval;
	
	SOELOG("START: SoEScreen::GetPalette(start:%i, count:%i, lpPE:%i)", Start, Count, lpPE);
	
	rval = m_lpDDPalette->GetEntries(0, Start, Count, lpPE);
	
	SOELOG("END: SoEScreen::GetPalette");
	
    return rval;
}

HRESULT SoEScreen::FillPalette(int r, int g, int b)
{
    HRESULT rval;
	PALETTEENTRY pe[256];
	
	SOELOG("START: SoEScreen::FillPalette(r:%i, g:%i, b:%i)", r, g, b);
	SOELOG("Get the color entries");
	
	rval = m_lpDDPalette->GetEntries(0, 0, 256, pe);
    if (FAILED(rval))
        return rval;
	
	SOELOG("Fill the color entries");
	
	for (int i = 1; i < 256; i++)
	{
		pe[i].peRed   = r;
		pe[i].peGreen = g;
		pe[i].peBlue  = b;
	}
	
	SOELOG("Set the color entries");
	rval = m_lpDDPalette->SetEntries(0, 0, 256, pe);
	
	SOELOG("END: SoEScreen::FillPalette");
	
    return rval;
}

HRESULT SoEScreen::GreyScale(void)
{
    HRESULT rval;
	PALETTEENTRY pe[256];
	int total, grey;
	
	SOELOG("START: SoEScreen::GreyScale");
	SOELOG("Get the color entries");
	
	rval = m_lpDDPalette->GetEntries(0, 0, 256, pe);
    if (FAILED(rval))
        return rval;
	
	SOELOG("Fill the color entries");
	
	for (int i = 1; i < 256; i++)
	{
		total = pe[i].peRed + pe[i].peGreen + pe[i].peBlue;
		grey = total / 3;
		
		pe[i].peRed   = grey;
		pe[i].peGreen = grey;
		pe[i].peBlue  = grey;
	}
	
	SOELOG("Set the color entries");
	
	rval = m_lpDDPalette->SetEntries(0, 0, 256, pe);
	
	SOELOG("END: SoEScreen::GreyScale");
	
    return rval;
}

HRESULT SoEScreen::FadeIn(int delay, LPPALETTEENTRY lpPE)
{
    HRESULT rval;
	PALETTEENTRY pe[256];
	DWORD timeNow, timeTarget;
	
	SOELOG("START: SoEScreen::FadeIn(delay:%i, lpPE:%i)", delay, lpPE);
	SOELOG("Get the color entries");
	
	rval = m_lpDDPalette->GetEntries(0, 0, 256, pe);
    if (FAILED(rval))
        return rval;
	
	SOELOG("Fill the color entries");
	
	for (int j = 1; j < 84; j++)
	{
		for (int i = 1; i < 256; i++)
		{
			// Red
			if (pe[i].peRed >(lpPE[i].peRed + 5))
				pe[i].peRed -= 3;
			else if (pe[i].peRed <(lpPE[i].peRed - 5))
				pe[i].peRed += 3;
			else 
				pe[i].peRed = lpPE[i].peRed;
			
			// Green
			if (pe[i].peGreen >(lpPE[i].peGreen + 5))
				pe[i].peGreen -= 3;
			else if (pe[i].peGreen <(lpPE[i].peGreen - 5))
				pe[i].peGreen += 3;
			else 
				pe[i].peGreen = lpPE[i].peGreen;
			
			// Blue
			if (pe[i].peBlue >(lpPE[i].peBlue + 5))
				pe[i].peBlue -= 3;
			else if (pe[i].peBlue <(lpPE[i].peBlue - 5))
				pe[i].peBlue += 3;
			else 
				pe[i].peBlue = lpPE[i].peBlue;
		}
		// Add a delay here for slower fades
		timeNow    = timeGetTime();
		timeTarget = timeNow + delay;
		
		while (timeNow < timeTarget)
			timeNow = timeGetTime();
		
		SOELOG("Set the color entries");
		rval = m_lpDDPalette->SetEntries(0, 0, 256, pe);
        if (FAILED(rval))
            return rval;
	}
	
	SOELOG("END: SoEScreen::FadeIn");
	
    return rval;
}

HRESULT SoEScreen::FadeOut(int delay)
{
    HRESULT rval;
	PALETTEENTRY pe[256];
	DWORD timeNow, timeTarget;
	
	SOELOG("START: SoEScreen::FadeOut(delay:%i)", delay);
	SOELOG("Get the color entries");
	
	rval = m_lpDDPalette->GetEntries(0, 0, 256, pe);
    if (FAILED(rval))
        return rval;
	
	SOELOG("Fill the color entries");
	
	for (int j = 1; j < 84; j++)
	{
		for (int i = 1; i < 256; i++)
		{
			// Red
			if (pe[i].peRed > 5)
				pe[i].peRed -= 3;
			else 
				pe[i].peRed = 0;
			
			// Green
			if (pe[i].peGreen > 5)
				pe[i].peGreen -= 3;
			else 
				pe[i].peGreen = 0;
			
			// Blue
			if (pe[i].peBlue > 5)
				pe[i].peBlue -= 3;
			else 
				pe[i].peBlue = 0;
		}
		// Add a delay here for slower fades
		timeNow    = timeGetTime();
		timeTarget = timeNow + delay;
		
		while (timeNow < timeTarget)
			timeNow = timeGetTime();
		
		SOELOG("Set the color entries");
		rval = m_lpDDPalette->SetEntries(0, 0, 256, pe);
        if (FAILED(rval))
            return rval;
	}
	
	SOELOG("END: SoEScreen::FadeOut");
	
    return rval;
}

HRESULT SoEScreen::FadeTo(int r, int g, int b, int delay)
{
    HRESULT rval;
	PALETTEENTRY pe[256];
	DWORD timeNow, timeTarget;
	
	SOELOG("START: SoEScreen::FadeTo(r:%i, g:%i, b:%i, delay:%i)", r, g, b, delay);
	SOELOG("Get the color entries");
	
	rval = m_lpDDPalette->GetEntries(0, 0, 256, pe);
    if (FAILED(rval))
        return rval;
	
	SOELOG("Fill the color entries");
	for (int j = 1; j < 84; j++)
	{
		for (int i = 1; i < 256; i++)
		{
			// Red
			if (pe[i].peRed > r + 5)
				pe[i].peRed -= 3;
			else if (pe[i].peRed < r - 5)
				pe[i].peRed += 3;
			else 
				pe[i].peRed = r;
			
			// Green
			if (pe[i].peGreen > g + 5)
				pe[i].peGreen -= 3;
			else if (pe[i].peGreen < g - 5)
				pe[i].peGreen += 3;
			else 
				pe[i].peGreen = g;
			
			// Blue
			if (pe[i].peBlue > b + 5)
				pe[i].peBlue -= 3;
			else if (pe[i].peBlue < b - 5)
				pe[i].peBlue += 3;
			else 
				pe[i].peBlue = b;
		}
		// Add a delay here for slower fades
		timeNow    = timeGetTime();
		timeTarget = timeNow + delay;
		
		while (timeNow < timeTarget)
			timeNow = timeGetTime();
		
		SOELOG("Get the color entries");
		rval = m_lpDDPalette->SetEntries(0, 0, 256, pe);
        if (FAILED(rval))
            return rval;
	}
	
	SOELOG("END: SoEScreen::FadeTo");
	
    return rval;
}

DWORD SoEScreen::GetNumberOfVideoModes(void)
{
    return m_NumberOfVideoModes;
}

BOOL SoEScreen::GetVideoModeInformation(DWORD Index, DWORD* Width, DWORD* Height, DWORD* BPP)
{
    SOE_VIDEOMODESSTRUCT * Modes;
	
	SOELOG("START: SoEScreen::GetVideoModeInformation(index:%i, width:%i, height:%i, bpp:%i)", Index, Width, Height, BPP);
	SOELOG("Modes Ptr: %i", m_VideoModes);
	
    Modes = m_VideoModes;
	
    if (Index < m_NumberOfVideoModes)
    {
		SOELOG("Index was < m_NumberOfVideoModes");
		
        while (Index > 0)
        {
            -- Index;
            Modes = (SOE_VIDEOMODESSTRUCT *)Modes->NextMode;
        }
        *Width   = Modes->Width;
        *Height  = Modes->Height;
        *BPP     = Modes->BPP;
		
		SOELOG("w:%i, h:%i, bpp:%i", Width, Height, BPP);
		SOELOG("END: SoEScreen::GetVideoModeInformation");
		
        return TRUE;
    }
    else
	{
		SOELOG("END: SoEScreen::GetVideoModeInformation");
        return FALSE;
	}
}

BOOL SoEScreen::CheckIfVideoModeExists(DWORD Width , DWORD Height , DWORD BPP)
{
    BOOL                    found;
    SOE_VIDEOMODESSTRUCT *  Modes;
	
	SOELOG("START: SoEScreen::CheckIfVideoModeExists(width:%i, height:%i, bpp:%i)", Width, Height, BPP);
	SOELOG("Modes Ptr: %i", m_VideoModes);
	
    Modes = m_VideoModes;
    found = FALSE;
	
    while (Modes->NextMode != NULL)
    {
		SOELOG("Try Mode: w:%i, h:%i, bpp:%i", Width, Height, BPP);
		
        if ((Modes->Width  == Width) &&
			(Modes->Height == Height) &&
			(Modes->BPP    == BPP))
        {
            found = TRUE;
			SOELOG("Found Mode!");
            break;
        }
		
        Modes = (SOE_VIDEOMODESSTRUCT *)Modes->NextMode;
    }
	
	SOELOG("END: SoEScreen::CheckIfVideoModeExists");
	
    return found;
}

HRESULT SoEScreen::WaitForVerticalBlank(void)
{
	SOELOG2("START: SoEScreen::WaitForVerticalBlank");
	
	return m_lpDD->WaitForVerticalBlank(DDWAITVB_BLOCKBEGIN , 0);
	
    SOELOG2("END: SoEScreen::WaitForVerticalBlank");
}

DWORD SoEScreen::MakeColor(DWORD Color , SoESurface * Surface)
{
    DDPIXELFORMAT   pf;
    unsigned int    i, j, 
		rshift, gshift, bshift, 
		rbits, gbits, bbits,
		RED , GREEN , BLUE,
		result;
    
	SOELOG2("START: SoEScreen::MakeColor(color:%i, surface:%i)", Color, Surface);
	
    // separate red,green.blue out of Color
    RED   = Color & 255;
    GREEN = (Color >> 8) & 255;
    BLUE  = (Color >> 16) & 255;
	
	SOELOG2("r:%i, g:%i, b:%i", RED, GREEN, BLUE);
	
    // get the destination surface pixel format
    ZeroMemory(&pf , sizeof(pf));
    pf.dwSize = sizeof(pf);
	
	SOELOG2("Get pixel format");
    if (Surface == NULL)
        GetBack()->GetDDS()->GetPixelFormat(&pf);
    else
        Surface->GetDDS()->GetPixelFormat(&pf);
	
    // convert the color
    if (pf.dwRGBBitCount>8)
    {
		SOELOG2("Mode depth > 8bit");
		
        j = (int) pf.dwRBitMask;
		rshift = 0;
        i = 1;
		while (!(i&j)) 
		{
			rshift++;
			i <<=1; 
		}
        rbits = 0;
		while (i&j) 
		{
			rbits++;
			i <<=1; 
		}
        j = (int) pf.dwGBitMask;
		gshift = 0;
        i = 1;
		while (!(i&j)) 
		{
			gshift++;
			i <<=1; 
		}
        gbits = 0;
		while (i&j) 
		{
			gbits++;
			i <<=1; 
		}
        j = (int) pf.dwBBitMask;
		bshift = 0;
        i = 1;
		while (!(i&j)) 
		{
			bshift++;
			i <<=1; 
		}
        bbits = 0;
		while (i&j) 
		{
			bbits++;
			i <<=1; 
		}
		
        result = (((RED << rshift) >> (8 - rbits)) & pf.dwRBitMask) |
			(((GREEN << gshift) >> (8 - gbits)) & pf.dwGBitMask) |
			(((BLUE << bshift) >> (8 - bbits)) & pf.dwBBitMask);
    }
    else
    {
        result = Color;
    }
	
	SOELOG2("Result Color: %i", result);
	SOELOG2("END: SoEScreen::MakeColor");
	
    return result;
}

HRESULT SoEScreen::EnumVideoModes(void)
{
    HRESULT rval;
	SOELOG("START: SoEScreen::EnumVideoModes");
	SOELOG("Do some init'ing");
	
    m_NumberOfVideoModes = 0;
    m_VideoModes = new SOE_VIDEOMODESSTRUCT;
	ZeroMemory(m_VideoModes, sizeof(SOE_VIDEOMODESSTRUCT));
	
	SOELOG("EnumDisplayModes");
	
#if DIRECTDRAW_VERSION >= SOE_DDVER
    rval = GetDD()->EnumDisplayModes(DDEDM_STANDARDVGAMODES, NULL, (LPVOID)m_VideoModes, EnumDisplayModesCallback);
#else
    rval = GetDD()->EnumDisplayModes(0, NULL, (LPVOID)m_VideoModes, EnumDisplayModesCallback);
#endif
    if (FAILED(rval))
	{
		SOELOG("END: SoEScreen::EnumVideoModes");
		return rval;
	}
	
	SOELOG("Get the number of video modes");
	
    // Find out how many different video modes were returned
    SOE_VIDEOMODESSTRUCT* Modes = (SOE_VIDEOMODESSTRUCT*)m_VideoModes;
	
    while (Modes->NextMode != NULL)
    {
        Modes = (SOE_VIDEOMODESSTRUCT *)Modes->NextMode;
		
		SOELOG("w:%i, h:%i, bpp:%i", Modes->Width, Modes->Height, Modes->BPP);
		
        ++ m_NumberOfVideoModes;
    }   
	
	SOELOG("END: SoEScreen::EnumVideoModes");
	
	return rval;
}

HRESULT SoEScreen::EnumDrivers(void)
{
    HRESULT rval;
	
	SOELOG("START: SoEScreen::EnumDrivers");
	
	m_NumberOfDrivers = 0;
	m_Drivers = new SOE_DRIVERSTRUCT;
    ZeroMemory(m_Drivers, sizeof(SOE_DRIVERSTRUCT));
	
#if DIRECTDRAW_VERSION >= SOE_DDVER
	SOELOG("DirectDrawEnumerateEx");
    rval = DirectDrawEnumerateEx(DDEnumCallback, (LPVOID)m_Drivers, DDENUM_ATTACHEDSECONDARYDEVICES | DDENUM_NONDISPLAYDEVICES);
#else
	SOELOG("DirectDrawEnumerate");
    rval = DirectDrawEnumerate(DDEnumCallback, (LPVOID)m_Drivers);
#endif
	
    if (FAILED(rval))
	{
		SOELOG("END: SoEScreen::EnumDrivers");
		return rval;
	}
	
	SOELOG("Get the number of drivers");
	
    // Find out how many different drivers were returned
	SOE_DRIVERSTRUCT *Drivers = m_Drivers;
	
    while (Drivers->NextDriver != NULL)
    {
        Drivers = Drivers->NextDriver;
		
		SOELOG("%s", Drivers->lpDesc);
		
        ++ m_NumberOfDrivers;
    }   
	
	SOELOG("END: SoEScreen::EnumDrivers");
	
	return rval;
}

HRESULT SoEScreen::SelectDriver(DWORD Index)
{	
	HRESULT rval;
	
	SOELOG("START: SoEScreen::SelectDriver(index:%i)", Index);
	SOELOG("NumDrivers: %i", m_NumberOfDrivers);
	
	// Check if the index is correct
	if (Index > m_NumberOfDrivers)
		return -1;
	
	SOELOG("Check to see if DX is already setup up a screen mode");
	
	// Check if direct draw has allready set up a screen mode
	// if so we must exit, else all the surfaces will have the wrong
	// direct draw interface
	if (GetFront())
		return -2;
	
	SOELOG("Find the driver");
	
	// Find the Indexed driver
	SOE_DRIVERSTRUCT* Drivers = m_Drivers;
	for (DWORD i = 0; i < Index; i++)
	{	
        Drivers = Drivers->NextDriver;
		
		if (Drivers == NULL) 
            return -4;
	}
	
	SOELOG("Destroy linked list with video mode infos");
	
	// destroy linked list with video modes informations
    SOE_VIDEOMODESSTRUCT* Modes , * Modes2;
    Modes = m_VideoModes;
	
    do
    {
        Modes2 = Modes;
        Modes  = (SOE_VIDEOMODESSTRUCT*)Modes->NextMode;
		if (Modes2 != NULL)
			delete Modes2;
    }
    while (Modes != NULL);
	
	SOELOG("Release DirectDraw");
	RELEASE(m_lpDD);
	
	// create direct draw object
#if DIRECTDRAW_VERSION >= SOE_DDVER
	SOELOG("Attempting a DirectDrawCreateEx");
	rval = DirectDrawCreateEx(NULL, (VOID**)&m_lpDD, IID_IDirectDraw7, NULL);
	if (FAILED(rval)) 
    {
        SOELOG("Failed to create an IID_IDirectDraw7 Interface!");
    }
#else
	SOELOG("Create DirectDraw");
	rval = DirectDrawCreate(Drivers->lpGUID, &m_lpDD, NULL);
	if (FAILED(rval)) 
        return rval;
#endif
	
	// Do video enumeration stuff, again
	EnumVideoModes();
	
	SOELOG("END: SoEScreen::SelectDriver");
	
	return 0;
}

void SoEScreen::AddSurfaceToList(SoESurface* lpSoES)
{
    m_SurfaceList.Add(lpSoES);
}

void SoEScreen::ReleaseAllSurfaces(void)
{
    BOOL bDeleteSurface = FALSE;
	
    m_SurfaceList.Clear(bDeleteSurface);
}

void SoEScreen::ReleaseSurfaceFromList(SoESurface* lpSoES)
{
    BOOL bDeleteSurface = FALSE;
	SoESurface* lpSoEs  = lpSoES;
	
    if ((lpSoES != GetFront()) &&(lpSoES != GetBack()))
        m_SurfaceList.Remove(lpSoEs, bDeleteSurface);
}

HRESULT SoEScreen::ReloadAllSurfaces(void)
{
    HRESULT rval;
    SoESurface* pSurface;
	
    // Restore any offScreen buffers that existed.
    for (pSurface = m_SurfaceList.GetFirst(); pSurface != NULL; pSurface = m_SurfaceList.Next(pSurface))
    {
        // Restore the surface first.
        rval = pSurface->Restore();
		
        // Reload the graphics to the new surface 
        rval = pSurface->ReLoadBitmap(pSurface->GetCaptureFileName());
		
        // Delete the file.
        remove(pSurface->GetCaptureFileName());
	}
	
    return rval;
}

HRESULT SoEScreen::SaveAllSurfaces(void)
{
    HRESULT rval;
    SoESurface* pSurface;
    DWORD index = 0;
    char filename[256] = {'\0'};
    // Restore any offScreen buffers that existed.
    for (pSurface = m_SurfaceList.GetFirst(); pSurface != NULL; pSurface = m_SurfaceList.Next(pSurface))
    {
        // Create the filename
        sprintf(filename, "surface%04d.bmp", pSurface->GetSurfaceId());
		
        // Restore the surface first.
        rval = pSurface->SaveAsBMP(filename);
		
        // Set the CaptureFileName for this surface.
        pSurface->SetCaptureFileName(filename);
	}
	
    return rval;
}

LONG SoEScreen::GetNextId(void)
{
    m_lastId++;
	
    return m_lastId; 
}

HRESULT SoEScreen::FadeToSource(WORD duration)
{
    HRESULT rval;
	float fadeInc;
	LONG ALPHA;
	DWORD fadeBegin;
	DWORD w, h;
	RECT sRect;
	SoESurface* tmp;
	
	if (m_BPP == 8)
	{
		// Sorry this does not support 8bpp mode.
		// use FadeIn()
	}
	else
	{
		// Create a tmp surface the size of the back buffer and
		// copy the back buffer to it.
		w = GetBack()->GetWidth();
		h = GetBack()->GetHeight();
		sRect.left   = 0;
		sRect.top    = 0;
		sRect.right  = w;
		sRect.bottom = h;
		
		tmp  = new SoESurface();
        if (tmp == NULL)
            return -1;
		
        rval = tmp->Create(this, w, h, SOEMEM_SYSTEMONLY);
        if (FAILED(rval))
        {
            SAFEDELETE(tmp);
            return rval;
        }
		
		rval = GetBack()->DrawBlk(tmp, 0, 0, &sRect);
        if (FAILED(rval))
        {
            SAFEDELETE(tmp);
            return rval;
        }
		
		ALPHA = 0;
		
		// Start with a completely black screen
		rval = GetBack()->Fill(0);
        if (FAILED(rval))
        {
            SAFEDELETE(tmp);
            return rval;
        }
		
		rval = Flip(TRUE, FALSE);
        if (FAILED(rval))
        {
            SAFEDELETE(tmp);
            return rval;
        }
		
		// Continue to brighten the display
		while (ALPHA < 256)
		{
			fadeBegin = timeGetTime();
			
			rval = tmp->Fade(GetBack(), 0, 0, &sRect, (WORD)ALPHA);
            if (FAILED(rval))
            {
                SAFEDELETE(tmp);
                return rval;
            }
			rval = Flip(TRUE, FALSE);
            if (FAILED(rval))
            {
                SAFEDELETE(tmp);
                return rval;
            }
			
			fadeInc = (float)(duration + 1) /(float)(timeGetTime() - fadeBegin);
			
			// Do some boundry checking
			if (fadeInc > 256)
				fadeInc = 256;
			else if (fadeInc < 1)
				fadeInc = 1;
			
			ALPHA +=(LONG)(512 / fadeInc);
		}
		
		// Just to be sure we have a pure copy of the source
		rval = tmp->DrawBlk(GetBack(), 0, 0, &sRect);
        if (FAILED(rval))
        {
            SAFEDELETE(tmp);
            return rval;
        }
		
		rval = Flip(TRUE, FALSE);
        if (FAILED(rval))
        {
            SAFEDELETE(tmp);
            return rval;
        }
		
		// Cleanup the surface
		SAFEDELETE(tmp);
		tmp = NULL;
	}
	
    return 0;
}

HRESULT SoEScreen::FadeToBlack(WORD duration)
{
    HRESULT rval;
	float fadeInc;
	LONG ALPHA;
	DWORD fadeBegin;
	SoESurface* tmp;
	RECT sRect;
    RECT* pRect;
	DWORD w, h;
	POINT pt;
	
	if (m_BPP == 8)
	{
		// Sorry this does not support 8bpp mode.
		// Use FadeOut()
	}
	else
	{
		// Create a tmp surface the size of the back buffer and
		// copy the front buffer to it.
		w = GetBack()->GetWidth();
		h = GetBack()->GetHeight();
		
		// Need to get the RECT defining the SOE app window
        if (IsFullScreen() == FALSE)
        {
			GetClientRect((HWND)m_hWnd, &sRect);
			pt.x = pt.y = 0;
			ClientToScreen((HWND)m_hWnd, &pt);
			OffsetRect(&sRect, pt.x, pt.y);
        }
        else
        {
            pRect = GetBack()->GetClipRect();
            sRect = *pRect;
        }
		
		// Create the temp buffer
        tmp  = new SoESurface();
        if (tmp == NULL)
            return -1;
		
        rval = tmp->Create(this, w, h, SOEMEM_SYSTEMONLY);
        if (FAILED(rval))
        {
            SAFEDELETE(tmp);
            return rval;
        }
		
		GetFront()->DrawBlk(tmp, 0, 0, &sRect);
		
		ALPHA = 256;
		sRect.left   = 0;
		sRect.top    = 0;
		sRect.right  = w;
		sRect.bottom = h;
		
		// Continue fading while decrementing the ALPHA value.
		while (ALPHA > 0)
		{
			fadeBegin = timeGetTime();
			
			tmp->Fade(GetBack(), 0, 0, &sRect, (WORD)ALPHA);
			Flip(TRUE, FALSE);
			
			fadeInc = (float)(duration + 1) /(float)(timeGetTime() - fadeBegin);
			
			// Do some boundry checking
			if (fadeInc > 256)
				fadeInc = 256;
			else if (fadeInc < 1)
				fadeInc = 1;
			
			ALPHA -=(LONG)(512 / fadeInc);
		}
		
		// Cleanup the surface
		delete tmp;
		tmp = NULL;
		
		// Just to be sure we have a pure BLACK screen
		GetBack()->Fill(0);
		Flip(TRUE, FALSE);
	}
	
    return 0;
}

void SoEScreen::SetSurfaceScreen(SoESurface* pSoES)
{
    pSoES->m_Screen = this;
}

void SoEScreen::SetSurfaceWidth(SoESurface* pSoES, int w)
{
    pSoES->m_PixelWidth = w;
}

void SoEScreen::SetSurfaceHeight(SoESurface* pSoES, int h)
{
    pSoES->m_PixelHeight = h;
}

void SoEScreen::SetSurfacePixelFormat(SoESurface* pSoES)
{
    GetRGBFormat(pSoES->GetDDS(), &(pSoES->m_PixelFormat));
}

void SoEScreen::SetSurfaceDDSPointer(SoESurface* pSoES, SOE_LPDIRECTDRAWSURFACE lpdds)
{
    pSoES->m_lpDDS = lpdds;
}

HRESULT SoEScreen::ChangeVideoMode(DWORD Width, DWORD Height, DWORD BPP, BOOL bVGA)
{
    HRESULT rval;
    SoESurface* pSurface;
    LONG numSurfaces = 0;
    LPDIRECTDRAWPALETTE lpDDPalette = NULL;
    LPDIRECTDRAWCLIPPER lpClipper   = NULL;
	SOE_DDSURFACEDESC ddsd;
	SOE_DDSURFACEDESC ddsdtmp;
	SOE_LPDIRECTDRAWSURFACE lpdds;
	
    // If we are currently in windowed mode then we must delete the back buffer
    if ((m_lpDDSBack != NULL) &&(m_bFullScreen != TRUE))
        SAFEDELETE(m_lpDDSBack);
	
    //  ...else in full screen deleting the front buffer will take care of the back as well
    if (m_lpDDSFront != NULL)
        SAFEDELETE(m_lpDDSFront);
	
    // Do they want Fullscreen or Windowed Mode?
    if ((BPP > 0) &&(m_lpDD != NULL))
    {
        rval = CreateFullScreen((HWND)GetWindowHandle(), Width, Height, BPP, bVGA);
		
        if (FAILED(rval))
            return rval;
    }
    else if (m_lpDD != NULL)
    {
        // Restore the display to the users normal desktop
        m_lpDD->RestoreDisplayMode();
		
        // Change the window to the right size
        // ????  Width and Height need to be adjusted ????
        SetWindowPos((HWND)m_hWnd, HWND_TOP, 0, 0, Width, Height, SWP_SHOWWINDOW);
		
        rval = CreateWindowed((HWND)GetWindowHandle(), Width, Height);
        SetWindowPos((HWND)m_hWnd, HWND_TOP, 0, 0, Width, Height, SWP_SHOWWINDOW);
		
        if (FAILED(rval))
            return rval;
    }
    else
    {
        return -1;
    }
	
    // Next we need to restore any offScreen buffers that existed.
    for (pSurface = m_SurfaceList.GetFirst(); pSurface != NULL; pSurface = m_SurfaceList.Next(pSurface))
    {
        // Setup the surface descriptor
        ZeroMemory(&ddsd, sizeof(ddsd));
        ddsd.dwSize   = sizeof(ddsd);
        ddsd.dwFlags  = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT;
        ddsd.dwWidth  = pSurface->GetWidth();
        ddsd.dwHeight = pSurface->GetHeight();
		
        // Was the old surface in video or system memory? ????
        pSurface->GetSurfaceDescriptor(&ddsdtmp);
        if (ddsdtmp.ddsCaps.dwCaps & DDSCAPS_VIDEOMEMORY)
            ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
        else
            ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY;
		
        // Did the old surface have texure enabled?
        if (pSurface->GetTextureEnabled() == TRUE)
            ddsd.ddsCaps.dwCaps |= DDSCAPS_TEXTURE;
		
        // Did the old surface have a palette attached to it?
        pSurface->GetDDS()->GetPalette(&lpDDPalette);
		
        // Did the old surface have a clipper attached to it?
        pSurface->GetDDS()->GetClipper(&lpClipper);
		
        // Delete the old DirectDraw Surface
        pSurface->GetDDS()->Release();
		
        // Create the new DirectDraw Surface
        lpdds = NULL;
        rval = m_lpDD->CreateSurface(&ddsd, &lpdds, NULL);
		
        // Did it fail the create?
        if (FAILED(rval))
            return rval;
		
        // No failure, set the pointer in the SoESurface object
        SetSurfaceDDSPointer(pSurface, lpdds);
		
        // Reset the pixel format structure
        SetSurfacePixelFormat(pSurface);
		
        // Set the colorKey to the upper-left hand corner pixel
        // This is a bit of a hack because the upper-left hand pixel
        // may not be the correct color key.
        pSurface->SetColorKey();
		
        // Reload the graphics to the new surface 
        pSurface->ReLoadBitmap();
		
        // Restore the palette if there was one.
        if (lpDDPalette != NULL)
            pSurface->GetDDS()->SetPalette(lpDDPalette);
		
        // Restore the clipper if there was one.
        if (lpClipper != NULL)
            pSurface->GetDDS()->SetClipper(lpClipper);
	}
	
	return 0;
}
