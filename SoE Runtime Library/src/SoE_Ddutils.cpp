/*

 * PROJECT     : Source of Evil Runtime Library v1.0 
 * PROGRAMMER  : Andreas Zahnleiter (Mafuba)    

*/

#undef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN

#include "SoE.h"
#include "SoE_Screen.h"
#include "SoE_Surface.h"

HRESULT CALLBACK EnumDisplayModesCallback(SOE_LPDDSURFACEDESC lpDDSurfaceDesc, LPVOID lpContext)
{
    SOE_VIDEOMODESSTRUCT* Modes;
	
    Modes = (SOE_VIDEOMODESSTRUCT*)lpContext;
	
    // browse through modes list until last element is reached
    while (Modes->NextMode != NULL)
        Modes = (SOE_VIDEOMODESSTRUCT*)Modes->NextMode;
	
    // fill in elements
    Modes->Width    = lpDDSurfaceDesc->dwWidth;
    Modes->Height   = lpDDSurfaceDesc->dwHeight;
    Modes->BPP      = lpDDSurfaceDesc->ddpfPixelFormat.dwRGBBitCount;
	
    // add new element to list
    Modes->NextMode = new SOE_VIDEOMODESSTRUCT;
    Modes           = (SOE_VIDEOMODESSTRUCT*)Modes->NextMode;
    ZeroMemory(Modes, sizeof(SOE_VIDEOMODESSTRUCT));
	
    return DDENUMRET_OK;
}

#if DIRECTDRAW_VERSION >= SOE_DDVER
BOOL WINAPI DDEnumCallback(GUID FAR *lpGUID, LPSTR lpDesc, LPSTR lpName, LPVOID lpContext, HMONITOR hm)
#else
BOOL WINAPI DDEnumCallback(GUID FAR *lpGUID, LPSTR lpDesc, LPSTR lpName, LPVOID lpContext)
#endif
{
	LPDIRECTDRAW lpDD;
	SOE_DRIVERSTRUCT  *Drivers = (SOE_DRIVERSTRUCT*)lpContext;
	
    // browse through driver list until last element is reached
    while (Drivers->NextDriver != NULL)
        Drivers = Drivers->NextDriver;
	
    // try and create a DD device using the specified GUID
    if (DirectDrawCreate(lpGUID, &lpDD, NULL) != DD_OK)
    {
		// failed, so ignore this device
		return DDENUMRET_OK;
	}
	
	// get caps of this DD driver
	Drivers->DDcaps.dwSize = sizeof(Drivers->DDcaps);
	Drivers->HELcaps.dwSize = sizeof(Drivers->DDcaps);
	HRESULT hres = lpDD->GetCaps(&Drivers->DDcaps, &Drivers->HELcaps);
	RELEASE(lpDD);
	
	if (hres != DD_OK)
	{
		// failed, so ignore this device
		return DDENUMRET_OK;
	}
	
	
    // fill elements
	
	if (lpGUID == NULL)
	{   
		Drivers->lpGUID = NULL;
	}
	else
	{
		Drivers->lpGUID = &Drivers->guid;
		memcpy(&Drivers->guid, lpGUID, sizeof(GUID));
	}
	strcpy(Drivers->lpName, lpName);
	strcpy(Drivers->lpDesc, lpDesc);
	
#if DIRECTDRAW_VERSION >= SOE_DDVER
	Drivers->hMonitor	= hm;
#endif
	
    // add new element to list
	Drivers->NextDriver = new SOE_DRIVERSTRUCT;
	ZeroMemory(Drivers->NextDriver, sizeof(SOE_DRIVERSTRUCT));
	
	return DDENUMRET_OK;
}

extern "C" SOE_LPDIRECTDRAWSURFACE DDLoadBitmap(SOE_LPDIRECTDRAW pdd, LPCSTR szBitmap, int dx, int dy, BOOL memoryType)
{
    HBITMAP                 hbm;
    BITMAP                  bm;
	SOE_DDSURFACEDESC       ddsd;
	SOE_LPDIRECTDRAWSURFACE pdds;
	
    //
    //  try to load the bitmap as a resource, if that fails, try it as a file
    //
    hbm = (HBITMAP)LoadImage(GetModuleHandle(NULL), szBitmap, IMAGE_BITMAP, dx, dy, LR_CREATEDIBSECTION);
	
    if (hbm == NULL)
        hbm = (HBITMAP)LoadImage(NULL, szBitmap, IMAGE_BITMAP, dx, dy, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
	
    if (hbm == NULL)
        return NULL;
	
    //
    // get size of the bitmap
    //
    GetObject(hbm, sizeof(bm), &bm);      // get size of bitmap
	
    //
    // create a DirectDrawSurface for this bitmap
    //
    ZeroMemory(&ddsd, sizeof(ddsd));
    ddsd.dwSize = sizeof(ddsd);
    ddsd.dwFlags = DDSD_CAPS | DDSD_HEIGHT |DDSD_WIDTH;
	
	switch (memoryType)
	{
		case SOEMEM_SYSTEMONLY:
			ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY;
			break;
			
		case SOEMEM_VIDEOONLY:
			ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_VIDEOMEMORY;
			break;
			
		case SOEMEM_VIDTHENSYS:
		default:
			ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
			break;
	}
	
    ddsd.dwWidth = bm.bmWidth;
    ddsd.dwHeight = bm.bmHeight;
	
    if (pdd->CreateSurface(&ddsd, &pdds, NULL) != DD_OK)
        return NULL;
	
    DDCopyBitmap(pdds, hbm, 0, 0, 0, 0);
	
    DeleteObject(hbm);
	
    return pdds;
}

SOE_LPDIRECTDRAWSURFACE DDCreateSurface(SOE_LPDIRECTDRAW pdd, DWORD width, DWORD height, SOE_LPDDSURFACEDESC  ddsd)
{
	BOOL                    free_ddsd;
	SOE_LPDIRECTDRAWSURFACE pdds;
	
	// Define what we want (surface description)
	if (ddsd == NULL)
	{
		free_ddsd = TRUE;
		
		ddsd = (SOE_LPDDSURFACEDESC) malloc(sizeof(SOE_DDSURFACEDESC));
		
		if (ddsd == NULL)
			return NULL;
	}
	else
	{
		free_ddsd = FALSE;
	}
	
	ZeroMemory(ddsd, sizeof(ddsd));
	
	ddsd->dwSize = sizeof(SOE_DDSURFACEDESC);
	ddsd->dwFlags = DDSD_CAPS | DDSD_HEIGHT |DDSD_WIDTH;
	ddsd->ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
	ddsd->dwWidth = width;
	ddsd->dwHeight = height;
	
	// If we can't create the surface, return NULL
	if (pdd->CreateSurface(ddsd, &pdds, NULL) != DD_OK)
	{
		return NULL;
	}
	
	if (free_ddsd)
	{
		free(ddsd);
	}
	
	// Success: return the surface
	return pdds;
}

SOE_LPDIRECTDRAWSURFACE DDLoadSizeBitmap(SOE_LPDIRECTDRAW pdd, LPCSTR szBitmap, int *dx, int *dy, BOOL memoryType)
{
	HBITMAP                 hbm;
	BITMAP                  bm;
	SOE_DDSURFACEDESC       ddsd;
	SOE_LPDIRECTDRAWSURFACE pdds;
	
	hbm = (HBITMAP)LoadImage(GetModuleHandle(NULL), szBitmap, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);
	
	if (hbm == NULL)
		hbm = (HBITMAP)LoadImage(NULL, szBitmap, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
	
	if (hbm == NULL)
		return NULL;
	
	//
	// get size of the bitmap
	//
	GetObject(hbm, sizeof(bm), &bm);
	
	//
	// create a DirectDrawSurface for this bitmap
	//
	ZeroMemory(&ddsd, sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);
	ddsd.dwFlags = DDSD_CAPS | DDSD_HEIGHT |DDSD_WIDTH;
	
	switch (memoryType)
	{
		case SOEMEM_SYSTEMONLY:
			ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY;
			break;
			
		case SOEMEM_VIDEOONLY:
			ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_VIDEOMEMORY;
			break;
			
		case SOEMEM_VIDTHENSYS:
		default:
			ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
			break;
	}
	
	ddsd.dwWidth = bm.bmWidth;
	ddsd.dwHeight = bm.bmHeight;
	
	if (pdd->CreateSurface(&ddsd, &pdds, NULL) != DD_OK)
	{
		return NULL;
	}
	DDCopyBitmap(pdds, hbm, 0, 0, 0, 0);
	
	DeleteObject(hbm);
	
	*dx = ddsd.dwWidth;
	*dy = ddsd.dwHeight;
	
	return pdds;
}

HRESULT DDReLoadBitmap(SOE_LPDIRECTDRAWSURFACE pdds, LPCSTR szBitmap)
{
    HBITMAP             hbm;
    HRESULT             hr;
	
    //
    //  try to load the bitmap as a resource, if that fails, try it as a file
    //
    hbm = (HBITMAP)LoadImage(GetModuleHandle(NULL), szBitmap, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);
	
    if (hbm == NULL)
        hbm = (HBITMAP)LoadImage(NULL, szBitmap, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
	
    if (hbm == NULL)
    {
        OutputDebugString("handle is null\n");
        return E_FAIL;
    }
	
    hr = DDCopyBitmap(pdds, hbm, 0, 0, 0, 0);
    if (hr != DD_OK)
    {
        OutputDebugString("ddcopybitmap failed\n");
    }
	
	
    DeleteObject(hbm);
    return hr;
}

extern "C" HRESULT DDCopyBitmap(SOE_LPDIRECTDRAWSURFACE pdds, HBITMAP hbm, int x, int y, int dx, int dy)
{
    HDC                 hdcImage;
    HDC                 hdc;
    BITMAP              bm;
    HRESULT             hr;
	SOE_DDSURFACEDESC   ddsd;
	
    if (hbm == NULL || pdds == NULL)
        return E_FAIL;
	
    //
    // make sure this surface is restored.
    //
    pdds->Restore();
	
    //
    //  select bitmap into a memoryDC so we can use it.
    //
    hdcImage = CreateCompatibleDC(NULL);
    if (!hdcImage)
        OutputDebugString("createcompatible dc failed\n");
    SelectObject(hdcImage, hbm);
	
    //
    // get size of the bitmap
    //
    GetObject(hbm, sizeof(bm), &bm);    // get size of bitmap
    dx = dx == 0 ? bm.bmWidth  : dx;    // use the passed size, unless zero
    dy = dy == 0 ? bm.bmHeight : dy;
	
    //
    // get size of surface.
    //
    ddsd.dwSize = sizeof(ddsd);
    ddsd.dwFlags = DDSD_HEIGHT | DDSD_WIDTH;
    pdds->GetSurfaceDesc(&ddsd);
	
    if ((hr = pdds->GetDC(&hdc)) == DD_OK)
    {
        StretchBlt(hdc, 0, 0, ddsd.dwWidth, ddsd.dwHeight, hdcImage, x, y, dx, dy, SRCCOPY);
        pdds->ReleaseDC(hdc);
    }
	
    DeleteDC(hdcImage);
	
    return hr;
}

extern "C" SOE_LPDIRECTDRAWPALETTE DDLoadPalette(SOE_LPDIRECTDRAW pdd, LPCSTR szBitmap)
{
    SOE_LPDIRECTDRAWPALETTE ddpal;
    int                 i;
    int                 n;
    int                 fh;
    HRSRC               h;
    LPBITMAPINFOHEADER  lpbi;
    PALETTEENTRY        ape[256];
    RGBQUAD *           prgb;
	
    //
    // build a 332 palette as the default.
    //
    for (i = 0; i < 256; i++)
    {
        ape[i].peRed   = (BYTE)(((i >> 5) & 0x07) * 255 / 7);
        ape[i].peGreen = (BYTE)(((i >> 2) & 0x07) * 255 / 7);
        ape[i].peBlue  = (BYTE)(((i >> 0) & 0x03) * 255 / 3);
        ape[i].peFlags = (BYTE)0;
    }
	
    //
    // get a pointer to the bitmap resource.
    //
    if (szBitmap &&(h = FindResource(NULL, szBitmap, RT_BITMAP)))
    {
        lpbi = (LPBITMAPINFOHEADER)LockResource(LoadResource(NULL, h));
        if (!lpbi)
            OutputDebugString("lock resource failed\n");
        prgb = (RGBQUAD*)((BYTE*)lpbi + lpbi->biSize);
		
        if (lpbi == NULL || lpbi->biSize < sizeof(BITMAPINFOHEADER))
            n = 0;
        else if (lpbi->biBitCount > 8)
            n = 0;
        else if (lpbi->biClrUsed == 0)
            n = 1 << lpbi->biBitCount;
        else
            n = lpbi->biClrUsed;
		
        //
        //  a DIB color table has its colors stored BGR not RGB
        //  so flip them around.
        //
        for (i = 0; i < n; i++)
        {
            ape[i].peRed   = prgb[i].rgbRed;
            ape[i].peGreen = prgb[i].rgbGreen;
            ape[i].peBlue  = prgb[i].rgbBlue;
            ape[i].peFlags = 0;
        }
    }
    else if (szBitmap &&(fh = _lopen(szBitmap, OF_READ)) != -1)
    {
        BITMAPFILEHEADER bf;
        BITMAPINFOHEADER bi;
		
        _lread(fh, &bf, sizeof(bf));
        _lread(fh, &bi, sizeof(bi));
        _lread(fh, ape, sizeof(ape));
        _lclose(fh);
		
        if (bi.biSize != sizeof(BITMAPINFOHEADER))
            n = 0;
        else if (bi.biBitCount > 8)
            n = 0;
        else if (bi.biClrUsed == 0)
            n = 1 << bi.biBitCount;
        else
            n = bi.biClrUsed;
		
        //
        //  a DIB color table has its colors stored BGR not RGB
        //  so flip them around.
        //
        for (i = 0; i < n; i++)
        {
            BYTE r = ape[i].peRed;
            ape[i].peRed  = ape[i].peBlue;
            ape[i].peBlue = r;
        }
    }
    else
    {
        // if the bitmapfile cannot be found, return NULL
        // if the bitmapfile name was NULL, create an own one
        if (szBitmap != NULL)
            return NULL;
    }
	
    pdd->CreatePalette(DDPCAPS_8BIT, ape, &ddpal, NULL);
	
    return ddpal;
}

extern "C" DWORD DDColorMatch(SOE_LPDIRECTDRAWSURFACE pdds, COLORREF rgb)
{
    COLORREF rgbT;
    HDC hdc;
    DWORD dw = CLR_INVALID;
    HRESULT hres;
	SOE_DDSURFACEDESC ddsd;
	
    //
    //  use GDI SetPixel to color match for us
    //
    if (rgb != CLR_INVALID && pdds->GetDC(&hdc) == DD_OK)
    {
        rgbT = GetPixel(hdc, 0, 0);             // save current pixel value
        SetPixel(hdc, 0, 0, rgb);               // set our value
        pdds->ReleaseDC(hdc);
    }
	
    //
    // now lock the surface so we can read back the converted color
    //
    ddsd.dwSize = sizeof(ddsd);
    while ((hres = pdds->Lock(NULL, &ddsd, 0, NULL)) == DDERR_WASSTILLDRAWING)
		;
	
    if (hres == DD_OK)
    {
        dw  = *(DWORD *)ddsd.lpSurface;                     // get DWORD
        dw &=(1 << ddsd.ddpfPixelFormat.dwRGBBitCount) - 1;  // mask it to bpp
        pdds->Unlock(NULL);
    }
	
    //
    //  now put the color that was there back.
    //
    if (rgb != CLR_INVALID && pdds->GetDC(&hdc) == DD_OK)
    {
        SetPixel(hdc, 0, 0, rgbT);
        pdds->ReleaseDC(hdc);
    }
	
    return dw;
}

extern "C" HRESULT DDSetColorKey(SOE_LPDIRECTDRAWSURFACE pdds, COLORREF rgb)
{
    DDCOLORKEY          ddck;
	
    ddck.dwColorSpaceLowValue  = DDColorMatch(pdds, rgb);
    ddck.dwColorSpaceHighValue = ddck.dwColorSpaceLowValue;
    return pdds->SetColorKey(DDCKEY_SRCBLT, &ddck);
}

BOOL GetRGBFormat(SOE_LPDIRECTDRAWSURFACE Surface, RGBFORMAT* rgb)
{
    BYTE shiftcount;
	SOE_DDSURFACEDESC ddsd;
	
	// get a surface description
    ddsd.dwSize = sizeof(ddsd);
    ddsd.dwFlags = DDSD_PIXELFORMAT;
	
    if (Surface->GetSurfaceDesc(&ddsd) != DD_OK)
        return FALSE;
	
	// Check if the surface is 4 or 8 bpp
	if (ddsd.ddpfPixelFormat.dwRGBBitCount <= 8)
	{
		rgb->bpp = ddsd.ddpfPixelFormat.dwRGBBitCount;
		
		rgb->depth.rgbRed      = 0;
		rgb->position.rgbRed   = 0;
		
		rgb->depth.rgbGreen    = 0;
		rgb->position.rgbGreen = 0;
		
		rgb->depth.rgbBlue     = 0;
		rgb->position.rgbBlue  = 0;
		
		return TRUE;
	}
	
	// Get the RED channel data for 15, 16, 24, or 32 bpp surface
    shiftcount = 0;
    while (!(ddsd.ddpfPixelFormat.dwRBitMask & 1) &&(shiftcount < 32))
    {
        ddsd.ddpfPixelFormat.dwRBitMask >>= 1;
        shiftcount++;
    }
	
	// depth is the highest possible value that this channel could
	// be set to. It is cast to a BYTE because 255 is the highest
	// value for all modes.
    rgb->depth.rgbRed = (BYTE) ddsd.ddpfPixelFormat.dwRBitMask;
	
	// Position represents where the color channel starts, bitwise, 
	// in the pixel. Another way of saying this that Position is 
	// eqaul to the number of zeros to the right of the channel's
	// bitmask.
    rgb->position.rgbRed = shiftcount;
	
    // Get the GREEN channel data for 15, 16, 24, or 32 bpp surface
    shiftcount = 0;
    while (!(ddsd.ddpfPixelFormat.dwGBitMask & 1) &&(shiftcount < 32))
    {
        ddsd.ddpfPixelFormat.dwGBitMask >>= 1;
        shiftcount++;
    }
    rgb->depth.rgbGreen = (BYTE)ddsd.ddpfPixelFormat.dwGBitMask;
    rgb->position.rgbGreen = shiftcount;
	
    // Get the BLUE channel data for 15, 16, 24, or 32 bpp surface
    shiftcount = 0;
    while (!(ddsd.ddpfPixelFormat.dwBBitMask & 1) &&(shiftcount < 32))
    {
        ddsd.ddpfPixelFormat.dwBBitMask >>= 1;
        shiftcount++;
    }
    rgb->depth.rgbBlue = (BYTE)ddsd.ddpfPixelFormat.dwBBitMask;
    rgb->position.rgbBlue = shiftcount;
	
	//  Determine the bpp for the surface. (15, 16, 24, or 32)
	if (ddsd.ddpfPixelFormat.dwFlags & DDPF_RGB)
	{
		if (ddsd.ddpfPixelFormat.dwRGBBitCount == 16)
		{
			if ((rgb->depth.rgbBlue + rgb->depth.rgbGreen + rgb->depth.rgbRed) == 93)
				rgb->bpp = 15;
			else
				rgb->bpp = 16;
		}
		else if (ddsd.ddpfPixelFormat.dwRGBBitCount == 24)
			rgb->bpp = 24;
		else if (ddsd.ddpfPixelFormat.dwRGBBitCount == 32)
			rgb->bpp = 32;
		else
			rgb->bpp = 0;
	}
	else
	{
		rgb->bpp = 0;
		return FALSE;
	}
	
	return TRUE;
}

#ifdef _M_ALPHA
BOOL __cdecl IsMMX(void)
{
	return FALSE;
}
#else
#ifdef _MSC_VER
BOOL __cdecl IsMMX(void)
{
    SYSTEM_INFO si;
    int nCPUFeatures = 0;
    GetSystemInfo(&si);
    if (si.dwProcessorType != PROCESSOR_INTEL_386 && si.dwProcessorType != PROCESSOR_INTEL_486)
    {
        try
        {
            __asm
            {
				; we must push/pop the registers << CPUID>>  writes to, as the
					; optimiser doesn't know about << CPUID>> , and so doesn't expect
					; these registers to change.
					push eax
					push ebx
					push ecx
					push edx
					
					; << CPUID>> 
					; eax = 0, 1, 2 -> CPU info in eax, ebx, ecx, edx
					mov eax, 1
					_emit 0x0f
					_emit 0xa2
					mov nCPUFeatures, edx
					
					pop edx
					pop ecx
					pop ebx
					pop eax
            }
        }
        catch (...) // just to be sure...
        {
			return false;
        }
    }
    return (nCPUFeatures & 0x00800000) != 0;
}
#else
typedef void(*ASMProc)(); 
BOOL __cdecl IsMMX(void)
{
    SYSTEM_INFO si;
    int nCPUFeatures = 0;
    char xMMXCode[2] = {0x0f, 0xa2};
    ASMProc function_xMMXCode;
	
    GetSystemInfo(&si);
    if (si.dwProcessorType != PROCESSOR_INTEL_386 && si.dwProcessorType != PROCESSOR_INTEL_486)
    {
        try
        {
            __asm
            {
                push eax
					push ebx
					push ecx
					push edx
					
					mov eax, 1
            }
            function_xMMXCode = (ASMProc)xMMXCode;
            function_xMMXCode();
            __asm 
			{
                mov nCPUFeatures, edx
					
					pop edx
					pop ecx
					pop ebx
					pop eax
            }
        }
        catch (...) // just to be sure...
        {
            return false;
        }
    }
    return (nCPUFeatures & 0x00800000) != 0;
}
#endif

#endif // #ifdef _M_ALPHA